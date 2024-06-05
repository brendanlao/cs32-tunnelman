#ifndef ACTOR_H
#define ACTOR_H

#include "GraphObject.h"

class StudentWorld;

class Actor : public GraphObject
{
public:
    //overloaded constructors
    Actor(StudentWorld* world, int imageID, int startX, int startY, Direction d, double size, int depth);
    Actor(StudentWorld* world, int imageID, int startX, int startY);
    //destructor
    virtual ~Actor();
    
    virtual void doSomething() = 0;
    bool isAnnoyed();
    void setAnnoyed();
    StudentWorld* getWorld();
    int getTicks();
    void incTicks();
    
    virtual bool isDistributable();
private:
    StudentWorld* m_world;
    bool m_annoyed;
    int m_ticks;
};

class Earth : public Actor
{
public:
    //constructor
    Earth(StudentWorld* world, int x, int y);
    //destructor
    virtual ~Earth();
    
    virtual void doSomething();
private:
    
};

//Sentients -------------------------------------------------------------------------------------------------------------------------------------

class Sentient : public Actor
{
public:
    Sentient(StudentWorld* world, int imageID, int x, int y, Direction d, int hp);
    virtual ~Sentient();
    
    int getHP();
    void decHP(int i);
    virtual void playDigSound(Direction d);
    void directionalMovement(Direction d);
private:
    int m_hp;
};

class TunnelMan : public Sentient
{
public:
    //constructor
    TunnelMan(StudentWorld* world);
    //destructor
    virtual ~TunnelMan();
    
    virtual void playDigSound(Direction d);
    virtual void doSomething();
    int getGold();
    int getSonar();
    int getSquirts();
    void addToInventory(int ID);
private:
    int m_numGold;
    int m_numSonar;
    int m_numSquirts;
};

class Squirt : public Actor
{
public:
    Squirt(StudentWorld* world, int x, int y, Direction d);
    virtual ~Squirt();
    
    virtual void doSomething();
private:
    int m_coord;
};

class Protester : public Sentient
{
public:
    Protester(StudentWorld* world, int imageID, int hp);
    virtual ~Protester();
    
    virtual void doSomething();
    virtual bool trackPlayer();
    void intersectionDirection();
    void resetNumSquares();
    Direction generateDirection();
    void getSquirted(int i);
    void stun(int ticks);
    virtual void bribe();
    bool decideToMove();
    bool facingTM();
    void yell();
    bool getState();
    void setLeave();

private:
    bool m_leaveField;
    int m_ticksBetweenMoves;
    int m_squaresToMove;
    int m_ticksSinceYell;
    int m_ticksSincePerpTurn;
    int m_ticksToStun;
    bool m_isStunned;
};

class RegularProtester : public Protester
{
public:
    RegularProtester(StudentWorld* world);
    virtual ~RegularProtester();

private:
};

class HardcoreProtester : public Protester
{
public:
    HardcoreProtester(StudentWorld* world);
    virtual ~HardcoreProtester();
    
    virtual void bribe();
    virtual bool trackPlayer();
private:
    int m_ticksToStare;
    int m_distanceToTrack;
};

//Distributables --------------------------------------------------------------------------------------------------------------------------------

class Distributable : public Actor
{
public:
    Distributable(StudentWorld* world, int imageID, int x, int y, Direction d, double depth);
    virtual ~Distributable();
    
    virtual bool isDistributable();
private:
    
};

class Boulder : public Distributable
{
public:
    Boulder(StudentWorld* world, int x, int y);
    virtual ~Boulder();
    
    virtual void doSomething();
private:
    bool m_waiting;
    bool m_playSound;
};

class Barrel : public Distributable
{
public:
    Barrel(StudentWorld* world, int x, int y);
    virtual ~Barrel();
    
    virtual void doSomething();
private:
    
};

class Gold : public Distributable
{
public:
    Gold(StudentWorld* world, int x, int y, bool state);
    virtual ~Gold();
    
    virtual void doSomething();
private:
    bool m_permanent;
};

//Spawnables ------------------------------------------------------------------------------------------------------------------------------------

class Spawnable : public Distributable
{
public:
    Spawnable(StudentWorld* world, int imageID, int x, int y);
    virtual ~Spawnable();
    
    void spawnableDoSomething(int score);
private:
    int m_maxTicks;
};

class Sonar : public Spawnable
{
public:
    Sonar(StudentWorld* world);
    virtual ~Sonar();
    
    virtual void doSomething();
private:
};

class Water : public Spawnable
{
public:
    Water(StudentWorld* world, int x, int y);
    virtual ~Water();
    
    virtual void doSomething();
private:
};

#endif // ACTOR_H
