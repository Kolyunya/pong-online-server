#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QUdpSocket>
#include <QVector>

class Peer;

class Message;

class NetworkManager :
    public QObject
{
    Q_OBJECT
    public:
        explicit NetworkManager ( void );
        void sendMessage ( Peer* peerPtr , const Message& message );
    private:
        void initializeServerCredentials ( void );
        void initializeTcpServer ( void );
        void initializeUdpSocket ( void );
        int getPeerIndex ( Peer* peerPtr ) const;
        Peer* getPeerByTcpSocket ( QTcpSocket* tcpSocketPtr ) const;
        Peer* getPeerByTcpAddress ( const QHostAddress& tcpAddress , quint16 tcpPort ) const;
        Peer* getPeerByUdpAddress ( const QHostAddress& udpAddress , quint16 udpPort ) const;
        void sendTcpMessage ( Peer* peerPtr , const Message& message );
        void sendUdpMessage ( Peer* peerPtr , const Message& message );
        QHostAddress serverAddress;
        quint16 serverPort;
        QTcpServer tcpServer;
        QUdpSocket udpSocket;
        QVector<Peer*> peers;
    private slots:
        void acceptPeer ( void );
        void deletePeer ( void );
        void processTcpInput ( void );
        void processUdpInput ( void );
    signals:
        void peerConnected ( Peer* peerPtr  );
        void peerDisconnected ( Peer* peerPtr );
        void messageReceived ( Peer* peerPtr , Message* messagePtr );
};

#endif // NETWORKMANAGER_HPP
