#include <QTcpSocket>
#include <QSettings>
#include <Peer/Peer.hpp>
#include <NetworkManager/NetworkManager.hpp>
#include <Message/Message/Delimiter/Delimiter.hpp>
#include <Message/Message/HandshakeRequest/HandshakeRequest.hpp>
#include <Message/Message/HandshakeResponse/HandshakeResponse.hpp>
#include <Message/Message/HandshakeAck/HandshakeAck.hpp>
#include <Message/MessageParser/MessageParser.hpp>

NetworkManager::NetworkManager ( void )
{
    this->initializeServerCredentials();
    this->initializeTcpServer();
    this->initializeUdpSocket();
}

void NetworkManager::sendMessage ( Peer* peerPtr , const Message& message )
{

    // Sends a message depending on it's transport type

    // Retrieve a transport type
    MessageTransport messageTransport = message.getTransport();

    // Send a TCP message
    if ( messageTransport == MessageTransport::TCP )
    {
        this->sendTcpMessage(peerPtr,message);
    }

    // Send a UDP message
    else if ( messageTransport == MessageTransport::UDP )
    {
        this->sendUdpMessage(peerPtr,message);
    }

}

void NetworkManager::initializeServerCredentials()
{

    // Initialize the settings object
    QSettings settings("PongOnlineServer.ini",QSettings::IniFormat);

    // Retrieve server address and port from the client settings
    QString serverAddressString = settings.value("Server/ServerAddress").toString();
    this->serverAddress = QHostAddress(serverAddressString);
    this->serverPort = settings.value("Server/ServerPort").toUInt();

}

void NetworkManager::initializeTcpServer ( void )
{

    // Listen for new connections to the TCP server
    QObject::connect
    (
        &this->tcpServer,
        SIGNAL(newConnection()),
        this,
        SLOT(acceptPeer())
    );

    // Launch the TCP server
    this->tcpServer.listen
    (
        this->serverAddress,
        this->serverPort
    );

}

void NetworkManager::initializeUdpSocket ( void )
{

    // Listen for the input on the UDP socket
    QObject::connect
    (
        &this->udpSocket,
        SIGNAL(readyRead()),
        this,
        SLOT(processUdpInput())
    );

    // Bind the UDP socket to a local address
    this->udpSocket.bind
    (
        this->serverAddress,
        this->serverPort
    );

}

int NetworkManager::getPeerIndex ( Peer* peerPtr ) const
{
    int peerIndex = this->peers.indexOf(peerPtr);
    return peerIndex;
}

Peer* NetworkManager::getPeerByTcpSocket ( QTcpSocket* tcpSocketPtr ) const
{
    for ( Peer* peerPtr : this->peers )
    {
        if ( peerPtr->tcpSocketPtr == tcpSocketPtr )
        {
            return peerPtr;
        }
    }
    return nullptr;
}

Peer* NetworkManager::getPeerByTcpAddress ( const QHostAddress& udpAddress , quint16 udpPort ) const
{
    for ( Peer* peerPtr : this->peers )
    {
        if
        (
            peerPtr->tcpSocketPtr->peerAddress() == udpAddress
                &&
            peerPtr->tcpSocketPtr->peerPort() == udpPort
        )
        {
            return peerPtr;
        }
    }
    return nullptr;
}

Peer* NetworkManager::getPeerByUdpAddress ( const QHostAddress& udpAddress , quint16 udpPort ) const
{
    for ( Peer* peerPtr : this->peers )
    {
        if
        (
            peerPtr->udpAddress == udpAddress
                &&
            peerPtr->udpPort == udpPort
        )
        {
            return peerPtr;
        }
    }
    return nullptr;
}

void NetworkManager::sendTcpMessage ( Peer* peerPtr , const Message& message )
{
    QTcpSocket* clientTcpSocketPtr = peerPtr->tcpSocketPtr;
    QByteArray messageData = message.getData();
    QByteArray messageDelimiter = Delimiter().getData();
    clientTcpSocketPtr->write(messageData);
    clientTcpSocketPtr->write(messageDelimiter);

    // Flush data from the output buffer to the actual socket
    clientTcpSocketPtr->flush();

}

void NetworkManager::sendUdpMessage ( Peer* peerPtr , const Message& message )
{
    QHostAddress clientUdpAddress = peerPtr->udpAddress;
    quint16 clientUdpPort = peerPtr->udpPort;
    QByteArray messageData = message.getData();
    this->udpSocket.writeDatagram(messageData,clientUdpAddress,clientUdpPort);

    // Flush data from the output buffer to the actual socket
    this->udpSocket.flush();

}

void NetworkManager::acceptPeer ( void )
{

    // Try to accept a client
    QTcpSocket* clientSocketPtr = this->tcpServer.nextPendingConnection();

    // Check if the client was effectively accepted
    if ( clientSocketPtr == nullptr )
    {

        // Something went wrong…
        return;

    }

    // The client was effectively accepted.

    // Listen for the client disconnection
    QObject::connect
    (
        clientSocketPtr,
        SIGNAL(disconnected()),
        this,
        SLOT(deletePeer())
    );

    // Listen for the client TCP input
    QObject::connect
    (
        clientSocketPtr,
        SIGNAL(readyRead()),
        this,
        SLOT(processTcpInput())
    );

    // Create a new peer
    Peer* peerPtr = new Peer();
    peerPtr->tcpSocketPtr = clientSocketPtr;
    this->peers.push_back(peerPtr);

    // Send the peer a hanshake message
    HandshakeRequest handshakeRequest
    (
        clientSocketPtr->peerAddress(),
        clientSocketPtr->peerPort()
    );
    this->sendMessage(peerPtr,handshakeRequest);

}

void NetworkManager::deletePeer ( void )
{
    QTcpSocket* tcpSocketPtr = reinterpret_cast<QTcpSocket*>(QObject::sender());
    Peer* peerPtr = this->getPeerByTcpSocket(tcpSocketPtr);
    int peerIndex = this->getPeerIndex(peerPtr);
    this->peers.removeAt(peerIndex);
    emit this->peerDisconnected(peerPtr);
    delete peerPtr;
}

void NetworkManager::processTcpInput ( void )
{

    QTcpSocket* tcpSocketPtr = reinterpret_cast<QTcpSocket*>(QObject::sender());
    Peer* peerPtr = this->getPeerByTcpSocket(tcpSocketPtr);

    // Retrieve all input data from the TCP socket
    QByteArray tcpInputData { peerPtr->tcpSocketPtr->readAll() };

    // Append all input data to the TCP socket local buffer
    peerPtr->tcpSocketBuffer.append(tcpInputData);

    // Check if the data received is a complete message
    QByteArray messageDelimiter { Delimiter().getData() };
    bool messageReceived { peerPtr->tcpSocketBuffer.contains(messageDelimiter) };

    // The whole message is not received yet
    if ( messageReceived == false )
    {

        // Nothing to process yet
        return;

    }

    // The whole message is already received

    // Extract the message
    int messageDelimiterIndex = peerPtr->tcpSocketBuffer.indexOf(messageDelimiter);
    QByteArray message = peerPtr->tcpSocketBuffer.left(messageDelimiterIndex);

    // Remove the message received with the delimiter
    peerPtr->tcpSocketBuffer.remove(0,messageDelimiterIndex+1);

    // The server processes TCP messages only from clients who already
    // made a handshake. Check if the peer has already made a handshake.
    if ( peerPtr->isConnected )
    {

        // Try to parse a message
        Message* messagePtr = MessageParser::parse(message,MessageTransport::TCP);

        // The message was parsed successfully
        if ( messagePtr != nullptr )
        {

            // Emit the message
            emit this->messageReceived(peerPtr,messagePtr);

        }

    }

    // Check if there are some more messages available
    emit tcpSocketPtr->readyRead();

}

void NetworkManager::processUdpInput ( void )
{

    quint64 messageSize = this->udpSocket.pendingDatagramSize();
    QByteArray message(messageSize,0);
    char* messageData = message.data();
    QHostAddress udpAddress;
    quint16 udpPort;
    this->udpSocket.readDatagram(messageData,messageSize,&udpAddress,&udpPort);

    try
    {

        // The only UDP message which is processed by network manager itself is the handshake
        HandshakeResponse handshakeResponse(message);
        QHostAddress tcpAddress = handshakeResponse.getAddress();
        quint16 tcpPort = handshakeResponse.getPort();

        // Send the client an ack
        this->udpSocket.writeDatagram(HandshakeAck().getData(),udpAddress,udpPort);

        // Get a pointer to the peer by it's TCP address
        Peer* peerPtr = this->getPeerByTcpAddress(tcpAddress,tcpPort);

        // Fill the peer info with its UDP address
        peerPtr->udpAddress = udpAddress;
        peerPtr->udpPort = udpPort;
        peerPtr->isConnected = true;

        // Tell the server that a peer successfully connected
        emit this->peerConnected(peerPtr);

        return;

    }
    catch ( ... ) { }

    // Get a pointer to the peer by it's UDP address
    Peer* peerPtr = this->getPeerByUdpAddress(udpAddress,udpPort);

    // Ignore UDP messages from unknown hosts
    if ( peerPtr == nullptr )
    {
        return;
    }

    // Try to parse a message
    Message* messagePtr = MessageParser::parse(message,MessageTransport::UDP);

    // The message was parsed successfully
    if ( messagePtr != nullptr )
    {

        // Emit the message
        emit this->messageReceived(peerPtr,messagePtr);

    }

}
