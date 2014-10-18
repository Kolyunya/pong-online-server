#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include <QVector>
#include <Game/Game/Game.hpp>
#include <Peer/Peer.hpp>

class Message;

class GameManager :
    public QObject
{
    Q_OBJECT
    public:
        int getGamesCount ( void ) const;
        void startGame ( Peer* playerPtr );
        void endGame ( Game* gamePtr );
        void endGame ( Peer* playerPtr );
        void setMoveDirection ( Peer* playerPtr , Message* messagePtr );
    private:
        Game* getGameAvaitingEnemy ( void ) const;
        Game* getPlayerGame ( Peer* playerPtr ) const;
        bool getHasPlayer ( Peer* playerPtr ) const;
        QVector<Game*> games;
    private slots:
        void processFieldRedraw ( void );
        void processScoreChange ( void );
        void processScoreLimitReach ( void );
    signals:
        void gameStarted ( Game* gamePtr );
        void gameEnded ( Game* gamePtr );
        void gameScoreChanged ( Game* gamePtr );
        void gameFieldRedrawn ( Game* gamePtr );
};

#endif // GAMEMANAGER_HPP
