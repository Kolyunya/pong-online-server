#include <QLine>
#include <QTimer>
#include <Game/Game/Game.hpp>
#include <Message/Message/FieldData/FieldData.hpp>
#include <Message/Message/ScoreData/ScoreData.hpp>

quint8 Game::scoreLimit = 5;

quint8 Game::updateTimeout = 10;

quint8 Game::transmitTimeout = 100;

quint32 Game::afterGoalPause = 5000;

quint32 Game::beforeStartPause = 5000;

Game::Game ( void )
{
    this->initializeUpdateTimer();
    this->initializeTransmitTimer();
    this->initializePlayers();
    this->configureField();
    this->configureScore();
}

bool Game::getIsComplete ( void ) const
{
    return this->players.at(1) != nullptr;
}

Peer* Game::getPlayer ( int playerId ) const
{
    return this->players.at(playerId);
}

int Game::getPlayerId ( Peer* playerPtr ) const
{

    if ( this->players.at(0) == playerPtr )
    {
        return 0;
    }

    else if ( this->players.at(1) == playerPtr )
    {
        return 1;
    }

    else
    {
        return -1;
    }

}

bool Game::getHasPlayer ( Peer* playerPtr ) const
{
    return this->getPlayerId(playerPtr) >= 0;
}

void Game::addPlayer ( Peer* playerPtr )
{

    if ( this->players.at(0) == nullptr )
    {
        this->players.at(0) = playerPtr;
    }

    else
    {
        this->players.at(1) = playerPtr;
    }

}

QPointF Game::getPlayerPosition ( Peer* playerPtr )
{
    int playerId = this->getPlayerId(playerPtr);
    if ( playerId == 0 )
    {
        return this->field.getPlayerPosition(0);
    }
    QLineF line
    (
        this->field.getPlayerPosition(1),
        this->field.getCenter()
    );
    QPointF playerPosition = line.pointAt(2);
    return playerPosition;
}

QPointF Game::getEnemyPosition ( Peer* playerPtr )
{
    int playerId = this->getPlayerId(playerPtr);
    if ( playerId == 0 )
    {
        return this->field.getPlayerPosition(1);
    }
    QLineF line
    (
        this->field.getPlayerPosition(0),
        this->field.getCenter()
    );
    QPointF playerPosition = line.pointAt(2);
    return playerPosition;
}

QPointF Game::getBallPosition ( Peer* playerPtr )
{
    int playerId = this->getPlayerId(playerPtr);
    if ( playerId == 0 )
    {
        return this->field.getBallPosition();
    }
    QLineF line
    (
        this->field.getBallPosition(),
        this->field.getCenter()
    );
    QPointF ballPosition = line.pointAt(2);
    return ballPosition;
}

FieldData Game::getFieldData ( Peer* playerPtr )
{
    return FieldData
    (
        this->getPlayerPosition(playerPtr),
        this->getEnemyPosition(playerPtr),
        this->getBallPosition(playerPtr)
    );
}

ScoreData Game::getScoreData ( Peer* playerPtr )
{
    int playerId = this->getPlayerId(playerPtr);
    int playerScore = playerId == 0 ? this->score.getValue(0) : this->score.getValue(1);
    int enemyScore = playerId == 0 ? this->score.getValue(1) : this->score.getValue(0);
    ScoreData scoreData(playerScore,enemyScore);
    return scoreData;
}

void Game::setPlayerDirection ( Peer* playerPtr , MoveDirection moveDirection )
{
    int playerId = this->getPlayerId(playerPtr);
    this->field.setPlayerMoveDirection(playerId,moveDirection);
}

void Game::start ( void )
{
    if ( this->getIsComplete() )
    {
        this->field.resetBall();
        this->updateTimer.start();
        this->transmitTimer.start();
    }
}

void Game::stop ( void )
{
    //this->updateTimer.stop();
    //this->transmitTimer.stop();
}

void Game::initializeUpdateTimer ( void )
{

    // The timer should run continiously
    this->updateTimer.setSingleShot(false);

    // The timer ticks every "Game::updateTimeout" milliseconds
    this->updateTimer.setInterval(Game::updateTimeout);

    QObject::connect
    (
        &this->updateTimer,
        SIGNAL(timeout()),
        &this->field,
        SLOT(update())
    );

}

void Game::initializeTransmitTimer ( void )
{

    // The timer should run continiously
    this->transmitTimer.setSingleShot(false);

    // The timer ticks every "Game::transmitTimeout" milliseconds
    this->transmitTimer.setInterval(Game::transmitTimeout);

    QObject::connect
    (
        &this->transmitTimer,
        SIGNAL(timeout()),
        this,
        SIGNAL(transmissionRequired())
    );

}

void Game::initializePlayers ( void )
{
    this->players.at(0) = nullptr;
    this->players.at(1) = nullptr;
}

void Game::configureField ( void )
{

}

void Game::configureScore ( void )
{

    // A player win upon gaining "Game::scoreLimit" points
    this->score.setLimit(Game::scoreLimit);

    // Listen to the score change singnal
    QObject::connect
    (
        &this->score,
        SIGNAL(changed()),
        this,
        SLOT(processScoreChange())
    );

    // Listen to the score finish event
    QObject::connect
    (
        &this->score,
        SIGNAL(reachedLimit()),
        this,
        SIGNAL(scoreReachedLimit())
    );

}

void Game::processScoreChange ( void )
{

    // Stop game updating
    this->stop();

    // Emit score change signal
    emit this->scoreChanged();

    // If score did not reach limit yet start resume a game after timeout
    if ( ! this->score.getLimitReached() )
    {
        QTimer::singleShot
        (
            Game::afterGoalPause,
            this,
            SLOT(start())
        );
    }

}
