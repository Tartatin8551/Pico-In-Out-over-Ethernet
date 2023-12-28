#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _server(this)
{
    ui->setupUi(this);
    _server.listen(QHostAddress::Any, 5000);
    connect(&_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    QList<QCheckBox*> Output = {ui->O0,ui->O1,ui->O2,ui->O3,ui->O4,ui->O5,ui->O6,ui->O7};
    for(int i=0; i<8; i++){
        connect(Output[i], SIGNAL(stateChanged(int)), this, SLOT(out(int)));

    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewConnection()
{
   QTcpSocket *clientSocket = _server.nextPendingConnection();
   connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
   connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

   _sockets.push_back(clientSocket);
}

void MainWindow::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
        _sockets.removeOne(sender);
    }
}

void MainWindow::onReadyRead()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray datas = sender->readAll();
    Data2Txt(datas);
    //qDebug() << datas;
}

void MainWindow::Data2Txt(QByteArray datas)
{
    uint8_t PortA = datas[0];

    QList<QCheckBox*> Input = {ui->I0,ui->I1,ui->I2,ui->I3,ui->I4,ui->I5,ui->I6,ui->I7};

    for(int i=0; i<8; i++){
        if((PortA&(1<<i))!=0){
            Input[i]->setChecked(true);
        }
        else {
            Input[i]->setChecked(false);
        }
    }

    int ADC1 = datas[2]*256+datas[3];
    int ADC2 = datas[4]*256+datas[5];

    ui->Adc1->setValue(ADC1);
    ui->Adc2->setValue(ADC2);

}

void MainWindow::out(int q)
{
    QList<QCheckBox*> Output = {ui->O0,ui->O1,ui->O2,ui->O3,ui->O4,ui->O5,ui->O6,ui->O7};
    QByteArray data[1];
    int a=0;
    for(int i=0; i<8; i++){
        if (Output[i]->isChecked()){
            a+=(1<<i);
        }
    }
    data->append(a);
    //qDebug() << data[0];
    for (QTcpSocket* socket : _sockets) {
        socket->write(data[0]);
    }
}

