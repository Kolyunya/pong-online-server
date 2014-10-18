#ifndef SERVER_HPP
#define SERVER_HPP

#include <QCoreApplication>
#include <QByteArray>
#include <NetworkManager/NetworkManager.hpp>
#include <Game/GameManager/GameManager.hpp>

class Peer;

class Server :
    public QCoreApplication
{
    Q_OBJECT
    public:
        explicit Server ( int& argc , char** argv );
    private:
        void initializeNetworkManager ( void );
        void initializeGameManager ( void );
        NetworkManager networkManager;
        GameManager gameManager;
    private slots:
        void peerConnected ( Peer* peerPtr );
        void peerDisconnected ( Peer* peerPtr );
        void messageReceived ( Peer* peerPtr , Message* messagePtr );
        void gameStarted ( Game* gamePtr );
        void gameEnded ( Game* gamePtr );
        void gameFieldRedrawn ( Game* gamePtr );
        void gameScoreChanged ( Game* gamePtr );
};

#endif // SERVER_HPP
