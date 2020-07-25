#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtConcurrent/QtConcurrent>

#include "roomshandler.h"
#include "tcpsockethandler.h"

class TcpServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerHandler(RoomsHandler* _roomsHandler, QObject *parent = nullptr);
    void initTcpServer();
    void acceptTcpConnection();
    void readTcpPacket();

private:
    static int sendTcpPacket(QTcpSocket*, QByteArray arr);
    static void sendHeader(QHostAddress receiverAddress, QByteArray data, uint16_t port);
    RoomsHandler* mRoomsHandler;
    QHostAddress mSenderAddress;
    QTcpSocket *mTcpServerConnection = nullptr;
    QTcpServer* mTcpServer;
    uint16_t mPort;
};

#endif // TCPSERVERHANDLER_H
