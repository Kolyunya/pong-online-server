#ifndef GAME_HPP
#define GAME_HPP

#include <array>
#include <QTimer>
#include <Peer/Peer.hpp>
#include <Game/GameBase/GameBase.hpp>
#include <MoveDirection/MoveDirection.hpp>

class Game :
    public GameBase
{
    Q_OBJECT
    friend class GameManager;
    public:
        Game ( void );
        bool getIsComplete ( void ) const;
        Peer* getPlayer ( int playerId ) const;
        int getPlayerId ( Peer* peerPtr ) const;
        bool getHasPlayer ( Peer* peerPtr ) const;
        void addPlayer ( Peer* playerPtr );
        QPointF getPlayerPosition ( Peer* playerPtr );
        QPointF getEnemyPosition ( Peer* playerPtr );
        QPointF getBallPosition ( Peer* playerPtr );
        FieldData getFieldData ( Peer* playerPtr );
        ScoreData getScoreData ( Peer* playerPtr );
        void setPlayerDirection ( Peer* playerPtr , MoveDirection moveDirection );
    private:
        void initializeUpdateTimer ( void );
        void initializeTransmitTimer ( void );
        void initializePlayers ( void );
        void configureField ( void );
        void configureScore ( void );
        QTimer updateTimer;
        QTimer transmitTimer;
        std::array<Peer*,2> players;
        static quint8 scoreLimit;
        static quint8 updateTimeout;
        static quint8 transmitTimeout;
        static quint32 afterGoalPause;
        static quint32 beforeStartPause;
    private slots:
        void start ( void );
        void stop ( void );
        void processScoreChange ( void );
    signals:
        void scoreChanged ( void );
        void scoreReachedLimit ( void );
        void transmissionRequired ( void );
};

#endif // GAME_HPP
