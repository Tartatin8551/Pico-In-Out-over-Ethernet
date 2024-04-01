#include <SPI.h>
#include <Ethernet.h>
#include <RPi_Pico_TimerInterrupt.h>

// Setup interval in µs between data transmit
#define INTERVAL_WRITE 100000   // 100ms

// Initialise Timers
RPI_PICO_Timer myTimer0(0);

#define MAC_ADDRESS { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }
#define SERVER_IP { 192, 168, 1, 15 }
#define SERVER_PORT 5000
#define DATA_SIZE 6

// Ethernet
byte mac[] = MAC_ADDRESS; // Define MAC address
IPAddress server(SERVER_IP);

// Initialize the Ethernet client library
EthernetClient client;

// Types of board : 0:Head 1:8E/8S, 2:16E, 3:16S
#define STACKS 1
//int Boards[STACKS] = {0,2,1,3}; // head + 3 extension boards
int Boards[STACKS] = {0};
int PIOH[18] = {0,1,4,5,6,7,8,9,10,11,12,13,14,15,22,28,26,27};
int PIOEX[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};


// Bytes of data
byte data[DATA_SIZE];
byte PortA = 0;
byte PortB = 0;
byte StatePortB = 0;
int Adc1 = 0;
int Adc2 = 0;

void setup() {
  Ethernet.init(17);          // Init WIZnet chip
  analogReadResolution(12);   // Set Analog resolution to 12 bit (Default = 10)
  PinIO();                    // Set Input/Output

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  //while (!Serial) {;/* wait for serial port to connect. Needed for native USB port only*/}

  Ethernet.begin(mac);
  //Serial.print("  DHCP assigned IP ");
  Serial.println(Ethernet.localIP());
  
  // Give the Ethernet shield a second to initialize:
  delay(1000);
  
  // Connection to the server
  client.connect(server, 5000);
  
  // Interval in microsecs
  myTimer0.attachInterruptInterval(INTERVAL_WRITE, Data_Transmit);
}

void loop() {
  // Infinite reconnecting loop
  while(!client.connected()) {
    if (!client.connect(server, 5000)) {delay(300);}  // 300 ms 
    delay(10);
  }

  for(int i=0; i<8; i++){bitWrite(PortA,i,digitalRead(PIOH[i]));}
  for(int i=0; i<8; i++){bitWrite(StatePortB,i,digitalRead(PIOH[i+8]));}
  Adc1 = analogRead(PIOH[16]);
  Adc2 = analogRead(PIOH[17]);

  while(client.available()){
    PortB = client.read();
    Serial.println(PortB);
  }

  if(StatePortB!=PortB){
    for(int i=8; i<16; i++){digitalWrite(PIOH[i],bitRead(PortB,i-8));}
  }
  delay(50);
}


// Running on core1
void setup1() {
  delay(5000);
}

void loop1() {
  data[0] = PortA;
  data[1] = StatePortB;
  data[2] = Adc1 >> 8;
  data[3] = Adc1 & 0xFF;
  data[4] = Adc2 >> 8;
  data[5] = Adc2 & 0xFF;
  //Serial.println(data);
}

void PinIO()
{
  for(int i=0; i<STACKS; i++){
    switch (Boards[i]) {
      case 0:
        for(int i=0; i<16; i++){pinMode(PIOH[i], i<8?INPUT_PULLDOWN:OUTPUT);}
        break;
      case 1:
        for(int i=0; i<16; i++){pinMode(PIOEX[i], i<8?INPUT_PULLDOWN:OUTPUT);}
        break;
      case 2:
        for(int i=0; i<16; i++){pinMode(PIOEX[i],INPUT_PULLDOWN);}
        break;
      case 3:
        for(int i=0; i<16; i++){pinMode(PIOEX[i],OUTPUT);}
        break;
    }
  }
}

// Timer interrupt service routines
bool Data_Transmit(struct repeating_timer *t){
  return client.write(data, DATA_SIZE);
}