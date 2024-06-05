#ifndef STUDENTWORLD_H
#define STUDENTWORLD_H

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <vector>

class Actor;
class TunnelMan;
class Earth;

const int MAX_X = 60;
const int MAX_Y = 60;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();

    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    TunnelMan* getTM();
    bool isDiggable(int x, int y, Actor::Direction d, bool isTM);
    bool nextToBoulder(int x, int y, Actor::Direction d);
    bool collidedwithTM(int x, int y, double distance);
    Protester* collidedwithProtester(int x, int y);
    double getDistance(int x1, int y1, int x2, int y2);
    void decBarrels();
    void decProtesters();
    void addActor(Actor* a);
    void makeDistributablesVisible(double distance);
    void moveTowardsExit(Protester* p);
    void sensePlayer(Protester* p);
    
    Actor::Direction findDirectiontoTM(int x, int y);
    bool validProtesterMove(int x, int y, Actor::Direction d);
    struct m_coordinate
    {
        bool operator<(const m_coordinate& lhs);
        int m_x, m_y;
        m_coordinate(int x, int y)
        {
            m_x = x;
            m_y = y;
        }
        m_coordinate()
        {
            m_x = -1;
            m_y = -1;
        }
        
    };
    
private:
    std::vector<Actor*> m_actors;
    TunnelMan* m_tm;
    Earth* m_earth[64][64];
    bool m_visited[64][64];
    
    int m_totalTicks;
    int m_numBarrels;
    int m_numProtesters;
    int m_targetProtesters;
    int m_ticksPerProtester;
    int m_probSpawnable;
    int m_probHardcore;
    
    void addDistributable(char c);
    bool checkWaterCoords(int x, int y);
    void spawnWater();
    void addProtester();
    void addSpawnable();
    bool checkDistance(int x, int y);
    void setDisplayText();
    void removeDeadObjects();
    std::string formatDisplayText(int level, int lives, int health, int squirts,
                                  int gold, int numBarrels, int sonar, int score);
};

#endif // STUDENTWORLD_H
