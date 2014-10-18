#include <Game/GameManager/GameManager.hpp>
#include <Message/Message/Move/Move.hpp>

int GameManager::getGamesCount ( void ) const
{
    return this->games.count();
}

void GameManager::startGame ( Peer* playerPtr )
{

    // The player is already playing
    if ( this->getHasPlayer(playerPtr) == true )
    {
        return;
    }

    // Try to find a game which is awaiting enemy
    Game* gamePtr = this->getGameAvaitingEnemy();

    // A game avaiting enemy was not found
    if ( gamePtr == nullptr )
    {

        // Start a new game
        gamePtr = new Game();

        QObject::connect
        (
            gamePtr,
            SIGNAL(transmissionRequired()),
            this,
            SLOT(processFieldRedraw())
        );

        QObject::connect
        (
            gamePtr,
            SIGNAL(scoreChanged()),
            this,
            SLOT(processScoreChange())
        );

        QObject::connect
        (
            gamePtr,
            SIGNAL(scoreReachedLimit()),
            this,
            SLOT(processScoreLimitReach())
        );

        // Add a game to the collection
        this->games.push_back(gamePtr);

    }

    // Join an player to the game
    gamePtr->addPlayer(playerPtr);

    // If a game is complete
    if ( gamePtr->getIsComplete() == true )
    {

        // Start a game in 5 seconds
        QTimer::singleShot(5000,gamePtr,SLOT(start()));

        // Emit a game started signale
        emit this->gameStarted(gamePtr);

    }

}

void GameManager::endGame ( Game* gamePtr )
{

    // Emit game ended signal
    emit this->gameEnded(gamePtr);

    // Delete the game
    int gameIndex = this->games.indexOf(gamePtr);
    this->games.removeAt(gameIndex);
    delete gamePtr;

}

void GameManager::endGame ( Peer* playerPtr )
{

    // Get peer's game
    Game* gamePtr = this->getPlayerGame(playerPtr);

    // The peer is not playing
    if ( gamePtr == nullptr )
    {
        return;
    }

    this->endGame(gamePtr);

}

void GameManager::setMoveDirection ( Peer* playerPtr , Message* messagePtr )
{
    Move* messageMovePtr = reinterpret_cast<Move*>(messagePtr);
    MoveDirection moveDirection = messageMovePtr->getDirection();
    Game* gamePtr = this->getPlayerGame(playerPtr);
    if ( gamePtr == nullptr )
    {
        return;
    }
    gamePtr->setPlayerDirection(playerPtr,moveDirection);
}

Game* GameManager::getGameAvaitingEnemy ( void ) const
{

    for ( Game* gamePtr : this->games )
    {
        if ( gamePtr->getIsComplete() == false )
        {
            return gamePtr;
        }
    }

    return nullptr;

}

Game* GameManager::getPlayerGame ( Peer* playerPtr ) const
{

    for ( Game* gamePtr : this->games )
    {
        if ( gamePtr->getHasPlayer(playerPtr) == true )
        {
            return gamePtr;
        }
    }

    return nullptr;

}

bool GameManager::getHasPlayer ( Peer* playerPtr ) const
{
    Game* gamePtr = this->getPlayerGame(playerPtr);
    bool hasPlayer = gamePtr != nullptr;
    return hasPlayer;
}

void GameManager::processFieldRedraw ( void )
{
    Game* gamePtr = reinterpret_cast<Game*>(QObject::sender());
    emit this->gameFieldRedrawn(gamePtr);
}

void GameManager::processScoreChange ( void )
{
    Game* gamePtr = reinterpret_cast<Game*>(QObject::sender());
    emit this->gameScoreChanged(gamePtr);
}

void GameManager::processScoreLimitReach ( void )
{
    Game* gamePtr = reinterpret_cast<Game*>(QObject::sender());
    this->endGame(gamePtr);
}

