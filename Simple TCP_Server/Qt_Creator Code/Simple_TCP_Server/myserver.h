#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>

class myserver : public QTcpServer
{
    Q_OBJECT
public:
    explicit myserver(QObject *parent = 0);
    void startServer();
signals:

public slots:

protected:
    void incomingConnection(qintptr socketDescriptor);

};

#endif // MYSERVER_H
