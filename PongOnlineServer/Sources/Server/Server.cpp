#include <QDebug>
#include <Server/Server.hpp>
#include <Peer/Peer.hpp>
#include <Message/MessageHeader/MessageHeader.hpp>
#include <Message/MessageParser/MessageParser.hpp>
#include <Message/Message/GameStarted/GameStarted.hpp>
#include <Message/Message/GameEnded/GameEnded.hpp>
#include <Message/Message/Move/Move.hpp>
#include <Message/Message/FieldData/FieldData.hpp>
#include <Message/Message/ScoreData/ScoreData.hpp>

Server::Server ( int& argc , char** argv ):
    QCoreApplication(argc,argv)
{

    this->initializeNetworkManager();
    this->initializeGameManager();
    qDebug() << "Air Hockey Server was initialized";

}

void Server::initializeNetworkManager ( void )
{

    QObject::connect
    (
        &this->networkManager,
        SIGNAL(peerConnected(Peer*)),
        this,
        SLOT(peerConnected(Peer*))
    );

    QObject::connect
    (
        &this->networkManager,
        SIGNAL(peerDisconnected(Peer*)),
        this,
        SLOT(peerDisconnected(Peer*))
    );

    QObject::connect
    (
        &this->networkManager,
        SIGNAL(messageReceived(Peer*,Message*)),
        this,
        SLOT(messageReceived(Peer*,Message*))
    );

}

void Server::initializeGameManager ( void )
{

    QObject::connect
    (
        &this->gameManager,
        SIGNAL(gameStarted(Game*)),
        this,
        SLOT(gameStarted(Game*))
    );

    QObject::connect
    (
        &this->gameManager,
        SIGNAL(gameEnded(Game*)),
        this,
        SLOT(gameEnded(Game*))
    );

    QObject::connect
    (
        &this->gameManager,
        SIGNAL(gameFieldRedrawn(Game*)),
        this,
        SLOT(gameFieldRedrawn(Game*))
    );

    QObject::connect
    (
        &this->gameManager,
        SIGNAL(gameScoreChanged(Game*)),
        this,
        SLOT(gameScoreChanged(Game*))
    );

}

void Server::peerConnected ( Peer* peerPtr )
{

    qDebug()
        << "Client connected"
        << peerPtr->tcpSocketPtr->peerAddress().toString()
        << peerPtr->tcpSocketPtr->peerPort();

}

void Server::peerDisconnected ( Peer* peerPtr )
{

    qDebug()
            << "Client disconnected"
            << peerPtr->tcpSocketPtr->peerAddress().toString()
            << peerPtr->tcpSocketPtr->peerPort();

    this->gameManager.endGame(peerPtr);

}

void Server::messageReceived ( Peer* peerPtr , Message* messagePtr )
{

    // Find out a message of which type was received
    MessageHeader messageHeader = messagePtr->getHeader();

    // Process a message depending on it's type
    switch ( messageHeader )
    {

        case MessageHeader::StartGameHeader :
        {
            this->gameManager.startGame(peerPtr);
            break;
        }

        case MessageHeader::MoveHeader :
        {
            this->gameManager.setMoveDirection(peerPtr,messagePtr);
            break;
        }

        // The server does not care about other message types. The following block
        // is here just to make a compiler warnings about unhandled enumeration
        // values disappear…
        default :
        {
            break;
        }

    }

    // Do not forget to free the memory allocated for the message when we
    // do not need it anymore
    delete messagePtr;

}

void Server::gameStarted ( Game* gamePtr )
{

    this->networkManager.sendMessage(gamePtr->getPlayer(0),GameStarted());
    this->networkManager.sendMessage(gamePtr->getPlayer(1),GameStarted());

}

void Server::gameEnded ( Game* gamePtr )
{

    if ( gamePtr->getPlayer(0) != nullptr )
    {
        this->networkManager.sendMessage(gamePtr->getPlayer(0),GameEnded());
    }

    if ( gamePtr->getPlayer(1) != nullptr )
    {
        this->networkManager.sendMessage(gamePtr->getPlayer(1),GameEnded());
    }

}

void Server::gameFieldRedrawn ( Game* gamePtr )
{

    Peer* firstPlayerPtr = gamePtr->getPlayer(0);
    FieldData firstPlayerData = gamePtr->getFieldData(firstPlayerPtr);
    Peer* secondPlayerPtr = gamePtr->getPlayer(1);
    FieldData secondPlayerData = gamePtr->getFieldData(secondPlayerPtr);
    this->networkManager.sendMessage(firstPlayerPtr,firstPlayerData);
    this->networkManager.sendMessage(secondPlayerPtr,secondPlayerData);

}

void Server::gameScoreChanged ( Game* gamePtr )
{

    Peer* firstPlayerPtr = gamePtr->getPlayer(0);
    ScoreData firstPlayerData = gamePtr->getScoreData(firstPlayerPtr);
    Peer* secondPlayerPtr = gamePtr->getPlayer(1);
    ScoreData secondPlayerData = gamePtr->getScoreData(secondPlayerPtr);
    this->networkManager.sendMessage(firstPlayerPtr,firstPlayerData);
    this->networkManager.sendMessage(secondPlayerPtr,secondPlayerData);

}
