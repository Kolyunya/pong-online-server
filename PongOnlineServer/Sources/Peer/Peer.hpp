#ifndef PEER_HPP
#define PEER_HPP

#include <QByteArray>
#include <QHostAddress>
#include <QtNetwork/QTcpSocket>

class Peer
{
    public:
        QTcpSocket* tcpSocketPtr;
        QHostAddress udpAddress;
        quint16 udpPort;
        QByteArray tcpSocketBuffer;
        bool isConnected;
};

#endif // PEER_HPP
