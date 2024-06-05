#include "Actor.h"
#include "StudentWorld.h"
using namespace std;

//Actor -----------------------------------------------------------------------------------------------------------------------------------------
Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, Direction d, double size, int depth)
      :GraphObject(imageID, startX, startY, d, size, depth)
{
    m_world = world;
    m_annoyed = false;
    m_ticks = 0;
}

Actor::Actor(StudentWorld* world, int imageID, int startX, int startY)
      :GraphObject(imageID, startX, startY)
{
    m_world = world;
    setVisible(false);
    m_ticks = 0;
}

Actor::~Actor()
{
    setVisible(false);
}

bool Actor::isAnnoyed()
{
    return m_annoyed;
}

void Actor::setAnnoyed()
{
    m_annoyed = true;
}

StudentWorld* Actor::getWorld()
{
    return m_world;
}

int Actor::getTicks()
{
    return m_ticks;
}

void Actor::incTicks()
{
    m_ticks++;
}

bool Actor::isDistributable()
{
    return false;
}

//Earth -----------------------------------------------------------------------------------------------------------------------------------------

//size .25, depth 3
Earth::Earth(StudentWorld* world, int x, int y)
      :Actor(world, TID_EARTH, x, y, right, .25, 3)
{
}

//earth does nothing
void Earth::doSomething(){}

Earth::~Earth()
{}

//Sentient --------------------------------------------------------------------------------------------------------------------------------------

Sentient::Sentient(StudentWorld* world, int imageID, int x, int y, Direction d, int hp)
         :Actor(world, imageID, x, y, d, 1.0, 0)
{
    setVisible(true);
    m_hp = hp;
}

Sentient::~Sentient()
{}

int Sentient::getHP()
{
    return m_hp;
}

void Sentient::decHP(int i)
{
    m_hp -= i;
}

//default: do nothing, only play dig sound for character
void Sentient::playDigSound(Direction d)
{}

void Sentient::directionalMovement(Direction d)
{
    if (d == none)
        return;
    else if (d == left)
    {
        setDirection(left);
        if (getX() > 0 && !getWorld()->nextToBoulder(getX(), getY(), left))
        {
            moveTo(getX() - 1, getY());
            playDigSound(left);                         //virtual function, only the tunnel man can play this sound
        }
    }
    else if (d == right)
    {
        setDirection(right);
        if (getX() < MAX_X && !getWorld()->nextToBoulder(getX(), getY(), right))
        {
            moveTo(getX() + 1, getY());
            playDigSound(right);
        }
    }
    else if (d == up)
    {
        setDirection(up);
        if (getY() < MAX_Y && !getWorld()->nextToBoulder(getX(), getY(), up))
        {
            moveTo(getX(), getY() + 1);
            playDigSound(up);
        }
    }
    else if (d == down)
    {
        setDirection(down);
        if (getY() > 0 && !getWorld()->nextToBoulder(getX(), getY(), down))
        {
            moveTo(getX(), getY() - 1);
            playDigSound(down);
        }
    }
}

//TunnelMan -------------------------------------------------------------------------------------------------------------------------------------

TunnelMan::TunnelMan(StudentWorld* world)
          :Sentient(world, TID_PLAYER, 30, MAX_X, right, 10)
{
    m_numGold = 0;
    m_numSonar = 1;
    m_numSquirts = 10;
}

void TunnelMan::playDigSound(Direction d)
{
    if (getWorld()->isDiggable(getX(), getY(), d, true))
        getWorld()->playSound(SOUND_DIG);
        
}

void TunnelMan::doSomething()
{
    if (isAnnoyed())
        return;
    int ch;
    if (getWorld()->getKey(ch) == true)
    {
        switch (ch)
        {
            case KEY_PRESS_LEFT:
                directionalMovement(left);
                break;
            case KEY_PRESS_RIGHT:
                directionalMovement(right);
                break;
            case KEY_PRESS_UP:
                directionalMovement(up);
                break;
            case KEY_PRESS_DOWN:
                directionalMovement(down);
                break;
            case KEY_PRESS_TAB:
                if (m_numGold > 0)
                {
                    getWorld()->addActor(new Gold(getWorld(), getX(), getY(), false));
                    m_numGold--;
                }
                break;
            case KEY_PRESS_SPACE:
                if (m_numSquirts > 0)
                {
                    //squirt add depends on the direction of the tunnel man
                    if (getDirection() == left)
                        getWorld()->addActor(new Squirt(getWorld(), getX() - 1, getY(), getDirection()));
                    else if (getDirection() == right)
                        getWorld()->addActor(new Squirt(getWorld(), getX() + 2, getY(), getDirection()));
                    else if (getDirection() == up)
                        getWorld()->addActor(new Squirt(getWorld(), getX(), getY() + 2, getDirection()));
                    else if (getDirection() == down)
                        getWorld()->addActor(new Squirt(getWorld(), getX(), getY() - 1, getDirection()));
                    getWorld()->playSound(SOUND_PLAYER_SQUIRT);
                    m_numSquirts--;
                }
                break;
            case KEY_PRESS_ESCAPE:
                setAnnoyed();
                break;
            case 'Z':
            case 'z':
                if (m_numSonar > 0)
                {
                    getWorld()->playSound(SOUND_SONAR);
                    getWorld()->makeDistributablesVisible(12);
                    m_numSonar--;
                }
        }
    }
}

TunnelMan::~TunnelMan()
{}

int TunnelMan::getGold()
{
    return m_numGold;
}

int TunnelMan::getSonar()
{
    return m_numSonar;
}

int TunnelMan::getSquirts()
{
    return m_numSquirts;
}

void TunnelMan::addToInventory(int ID)
{
    if (ID == TID_GOLD)
        m_numGold++;
    else if (ID == TID_SONAR)
        m_numSonar += 2;
    else if (ID == TID_WATER_POOL)
        m_numSquirts += 5;
}

//Squirt ----------------------------------------------------------------------------------------------------------------------------------------

Squirt::Squirt(StudentWorld* world, int x, int y, Direction d)
       :Actor(world, TID_WATER_SPURT, x, y, d, 1, 1)
{
    setVisible(true);
    m_coord = 4;
}

Squirt::~Squirt()
{}

void Squirt::doSomething()
{
    if (isAnnoyed())
        return;
    //m_coord keeps track of how many blocks the squirt has left to move
    if (m_coord == 0)
        setAnnoyed();
    
    //case hit protester
    Protester* a = getWorld()->collidedwithProtester(getX(), getY());
    if (a != nullptr)
    {
        a->getSquirted(2);
        setAnnoyed();
        if (a->getState())
        {
            getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
            if (a->getID() == TID_PROTESTER)
                getWorld()->increaseScore(100);
            else
                getWorld()->increaseScore(250);
        }
        else
            getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
    }
    //case keep moving
    Direction d = getDirection();
    if (d == left)
    {
        if (getX() > 0 && !getWorld()->isDiggable(getX() - 1, getY(), d, false)
            && !getWorld()->nextToBoulder(getX(), getY(), d))
        {
            moveTo(getX() - 1, getY());
            m_coord--;
        }
        else
            setAnnoyed();
    }
    else if (d == right)
    {
        if (getX() < MAX_X && !getWorld()->isDiggable(getX() + 1, getY(), d, false)
            && !getWorld()->nextToBoulder(getX(), getY(), d))
        {
            moveTo(getX() + 1, getY());
            m_coord--;
        }
        else
            setAnnoyed();
    }
    else if (d == up)
    {
        if (getY() < MAX_Y && !getWorld()->isDiggable(getX(), getY() + 1, d, false)
            && !getWorld()->nextToBoulder(getX(), getY(), d))
        {
            moveTo(getX(), getY() + 1);
            m_coord--;
        }
        else
            setAnnoyed();
    }
    else if (d == down)
    {
        if (getY() > 0 && !getWorld()->isDiggable(getX(), getY() - 1, d, false)
            && !getWorld()->nextToBoulder(getX(), getY(), d))
        {
            moveTo(getX(), getY() - 1);
            m_coord--;
        }
        else
            setAnnoyed();
    }
}

//Protester -------------------------------------------------------------------------------------------------------------------------------------

Protester::Protester(StudentWorld* world, int imageID, int hp)
          :Sentient(world, imageID, MAX_X, MAX_Y, left, hp)
{
    m_leaveField = false;
    m_ticksBetweenMoves = max((unsigned int)0, 3 - getWorld()->getLevel()/4);
    resetNumSquares();
    //placeholder value, just has to be above 15
    m_ticksSinceYell = 1000;
    //placeholder value, just has to be above 200
    m_ticksSincePerpTurn = 1000;
    m_ticksToStun = max((unsigned int)50, 100 - getWorld()->getLevel() * 10);
    m_isStunned = false;
}

bool Protester::trackPlayer()
{
    return false;
}

Actor::Direction Protester::generateDirection()
{
    int choice = rand() % 4;
    if (choice == 0)
        return left;
    else if (choice == 1)
        return right;
    else if (choice == 2)
        return up;
    else                                //in this case, choice is 3
        return down;
}

void Protester::resetNumSquares()
{
    m_squaresToMove = rand() % 53 + 8;      //8 and 60 inclusive
}

Protester::~Protester()
{}

void Protester::getSquirted(int i)
{
    decHP(i);
    if (getHP() <= 0)
    {
        setLeave();
        m_ticksBetweenMoves = 0;
    }
    if (!m_isStunned)                       //only stun if they are not already stunned
    {
        m_isStunned = true;
        stun(m_ticksToStun);
    }
}

void Protester::stun(int ticks)
{
    m_ticksBetweenMoves += ticks;
}

void Protester::bribe()
{
    setLeave();
}

bool Protester::decideToMove()
{
    if (m_ticksBetweenMoves == 0)
    {
        m_ticksBetweenMoves = max((unsigned int)0, 3 - getWorld()->getLevel()/4);
        return true;
    }
    else
    {
        m_ticksBetweenMoves--;
        return false;
    }
        
}

void Protester::doSomething()
{
    if (isAnnoyed())                                                        //see if alive
        return;
    
    if (m_leaveField)                                                       //see if time to leave
    {
        if (getX() == MAX_X && getY() == MAX_Y)
        {
            setAnnoyed();
            getWorld()->decProtesters();
        }
    }
    
    if (!decideToMove())                                                    //see if in rest state
        return;
    else
    {
        if (m_leaveField)
        {
            getWorld()->moveTowardsExit(this);
            return;
        }
        m_squaresToMove--;
        m_ticksSincePerpTurn++;
        m_ticksSinceYell++;
        m_isStunned = false;
    }
    
    if (getWorld()->collidedwithTM(getX(), getY(), 4) && facingTM()         //time to yell
        && (getX() == getWorld()->getTM()->getX()
            || getY() == getWorld()->getTM()->getY()))
    {
        yell();
        return;
    }
    
    //this is a virtual function, only hardcore protesters will do something
    if (trackPlayer())
    {
        return;
    }
    
    Direction d = getWorld()->findDirectiontoTM(getX(), getY());            //move to TM if possible
    if (d != none)
    {
        directionalMovement(d);
        m_squaresToMove = 0;
        return;
    }
    
    
    if (m_squaresToMove <= 0)
    {
        Direction d = generateDirection();
        int breakTheGlass = 0;
        for (;;)
        {
            breakTheGlass++;                                                            //generate random direction until it is valid
            if (d == left && !getWorld()->validProtesterMove(getX() - 1, getY(), d))
                d = generateDirection();
            else if (d == right && !getWorld()->validProtesterMove(getX() + 1, getY(), d))
                d = generateDirection();
            else if (d == up && !getWorld()->validProtesterMove(getX(), getY() + 1, d))
                d = generateDirection();
            else if (d == down && !getWorld()->validProtesterMove(getX(), getY() - 1, d))
                d = generateDirection();
            else
            {
                setDirection(d);
                resetNumSquares();
                break;
            }
            if (breakTheGlass > 50)                                             //BREAK THE GLASS!! incase someone goes wrong
            {
                resetNumSquares();
                break;
            }
        }
    }
    else if (m_ticksSincePerpTurn > 200)                                        //changes the direction if at intersection
        intersectionDirection();
    
    directionalMovement(getDirection());
                                                                                //if the protester encounters a blockage
    if (getDirection() == left && !getWorld()->validProtesterMove(getX() - 1, getY(), getDirection()))
        m_squaresToMove = 0;
    else if (getDirection() == right && !getWorld()->validProtesterMove(getX() + 1, getY(), getDirection()))
        m_squaresToMove = 0;
    else if (getDirection() == up && !getWorld()->validProtesterMove(getX(), getY() + 1, getDirection()))
        m_squaresToMove = 0;
    else if (getDirection() == down && !getWorld()->validProtesterMove(getX(), getY() - 1, getDirection()))
        m_squaresToMove = 0;
             
}

void Protester::intersectionDirection()
{
    if (getDirection() == left || getDirection() == right)
    {
        bool turnUp = getWorld()->validProtesterMove(getX(), getY() + 1, up);
        bool turnDown = getWorld()->validProtesterMove(getX(), getY() - 1, down);
        if (!turnUp && !turnDown)
            return;
        else if (turnUp && !turnDown)               //can turn up but not down
        {
            setDirection(up);
            m_ticksSincePerpTurn = 0;
            resetNumSquares();
            return;
        }
        else if (!turnUp && turnDown)               //can turn down but not up
        {
            setDirection(down);
            m_ticksSincePerpTurn = 0;
            resetNumSquares();
            return;
        }
        else                                        //else, both work so choose randomly
        {
            int choice = rand() % 2;
            if (choice == 0)
                setDirection(up);
            else if (choice == 1)
                setDirection(down);
            m_ticksSincePerpTurn = 0;
            resetNumSquares();
            return;
        }
        
    }
    else if (getDirection() == up || getDirection() == down)
    {
        bool turnLeft = getWorld()->validProtesterMove(getX() - 1, getY(), left);
        bool turnRight = getWorld()->validProtesterMove(getX() + 1, getY(), right);
        if (!turnLeft && !turnRight)
            return;
        else if (turnLeft && !turnRight)                //can turn left but not right
        {
            setDirection(left);
            m_ticksSincePerpTurn = 0;
            resetNumSquares();
            return;
        }
        else if (!turnLeft && turnRight)                //can turn right but not left
        {
            setDirection(right);
            m_ticksSincePerpTurn = 0;
            resetNumSquares();
            return;
        }
        else                                            //both work, so choose randomly
        {
            int choice = rand() % 2;
            if (choice == 0)
                setDirection(left);
            else if (choice == 1)
                setDirection(right);
            m_ticksSincePerpTurn = 0;
            resetNumSquares();
            return;
        }
    }
}

bool Protester::facingTM()
{
    if (getDirection() == left)
        return getX() >= getWorld()->getTM()->getX();           //if facing left, protester should be on the right of TM
    else if (getDirection() == right)
        return getX() <= getWorld()->getTM()->getX();           //if facing right, protester should be on the left of TM
    else if (getDirection() == up)
        return getY() <= getWorld()->getTM()->getY();           //if facing up, protester should be below TM
    else if (getDirection() == down)
        return getY() >= getWorld()->getTM()->getY();           //if facing down, protester should be above TM
    
    return false;
}

void Protester::yell()
{
    if (m_ticksSinceYell >= 15)
    {
        getWorld()->getTM()->decHP(2);
        m_ticksSinceYell = 0;
        getWorld()->playSound(SOUND_PROTESTER_YELL);
        if (getWorld()->getTM()->getHP() <= 0)
        {
            getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
            getWorld()->getTM()->setAnnoyed();
        }
    }
    else
        return;
}

bool Protester::getState()
{
    return m_leaveField;
}

void Protester::setLeave()
{
    m_leaveField = true;
}

//Regular Protester

RegularProtester::RegularProtester(StudentWorld* world)
                 :Protester(world, TID_PROTESTER, 5)
{
    
}

RegularProtester::~RegularProtester()
{}

//Hardcore Protester

HardcoreProtester::HardcoreProtester(StudentWorld* world)
                  :Protester(world, TID_HARD_CORE_PROTESTER, 20)
{
    m_ticksToStare = max((unsigned int)50, 100 - getWorld()->getLevel() * 10);
    m_distanceToTrack = 16 + getWorld()->getLevel() * 2;
}

HardcoreProtester::~HardcoreProtester()
{}

void HardcoreProtester::bribe()
{
    stun(m_ticksToStare);
}

bool HardcoreProtester::trackPlayer()
{
    if (getWorld()->getDistance(getX(), getY(), getWorld()->getTM()->getX(), getWorld()->getTM()->getY()) < m_distanceToTrack)
    {
        getWorld()->sensePlayer(this);
        return true;
    }
    return false;
}

//Distributable ---------------------------------------------------------------------------------------------------------------------------------

Distributable::Distributable(StudentWorld* world, int imageID, int x, int y, Direction d, double depth)
              :Actor(world, imageID, x, y, d, 1.0, depth)
{}

Distributable::~Distributable()
{}

bool Distributable::isDistributable()
{
    return true;
}

//Boulder ---------------------------------------------------------------------------------------------------------------------------------------

Boulder::Boulder(StudentWorld* world, int x, int y)
        :Distributable(world, TID_BOULDER, x, y, down, 1)
{
    setVisible(true);
    m_waiting = false;
    m_playSound = true;
}

Boulder::~Boulder(){}

void Boulder::doSomething()
{
    if (isAnnoyed())
        return;
    if (!getWorld()->isDiggable(getX(), getY() - 1, down, false))
    {
        m_waiting = true;
        incTicks();
        if (getTicks() >= 30)                                                   //if its time to fall, fall
        {
            if (m_playSound)                                                    //only play the sound ONCE
            {
                getWorld()->playSound(SOUND_FALLING_ROCK);
                m_playSound = false;
            }
            moveTo(getX(), getY() - 1);                                         //fall code
            if (getWorld()->collidedwithTM(getX(), getY(), 3.5))                //annoy the TM
            {
                getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
                getWorld()->getTM()->setAnnoyed();
            }
            
            Protester* p = getWorld()->collidedwithProtester(getX(), getY());
            if (p != nullptr && !p->getState())                                 //annoy protester if applicable
            {
                p->setLeave();
                getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
                getWorld()->increaseScore(500);
            }
            
            if (getWorld()->isDiggable(getX(), getY() - 1, down, false)
                || getY() == 0 || getWorld()->nextToBoulder(getX(), getY(), Actor::down))
                setAnnoyed();
        }
    }
}

//Barrel ---------------------------------------------------------------------------------------------------------------------------------------

Barrel::Barrel(StudentWorld* world, int x, int y)
       :Distributable(world, TID_BARREL, x, y, right, 2)
{
    setVisible(false);
};

Barrel::~Barrel(){}

void Barrel::doSomething()
{
    if (isAnnoyed())
        return;
    if (!isVisible() && getWorld()->collidedwithTM(getX(), getY(), 5))
    {
        setVisible(true);
        return;
    }
    if (getWorld()->collidedwithTM(getX(), getY(), 3.5))
    {
        getWorld()->increaseScore(1000);
        getWorld()->playSound(SOUND_FOUND_OIL);
        setAnnoyed();
        getWorld()->decBarrels();
    }
}

//Gold ---------------------------------------------------------------------------------------------------------------------------------------

Gold::Gold(StudentWorld* world, int x, int y, bool state)
       :Distributable(world, TID_GOLD, x, y, right, 2)
{
    m_permanent = state;
    if (m_permanent)
        setVisible(false);
    else
        setVisible(true);
}

Gold::~Gold()
{}

void Gold::doSomething()
{
    if (isAnnoyed())
        return;
    if (!isVisible() && getWorld()->collidedwithTM(getX(), getY(), 5))
    {
        setVisible(true);
        return;
    }
    if (!m_permanent)
    {
        incTicks();
        if (getTicks() > 100)
            setAnnoyed();
        Protester* a = getWorld()->collidedwithProtester(getX(), getY());
        if (a != nullptr && !a->getState())                     //only do something if the protester is not in a leave the field state
        {
            if (a->getID() == TID_PROTESTER)
                getWorld()->increaseScore(25);
            else
                getWorld()->increaseScore(50);
            getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
            a->bribe();
            setAnnoyed();
            return;
        }
    }

    if (m_permanent && getWorld()->collidedwithTM(getX(), getY(), 3.5))
    {
        setAnnoyed();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(10);
        getWorld()->getTM()->addToInventory(getID());
        return;
    }
}

Spawnable::Spawnable(StudentWorld* world, int imageID, int x, int y)
          :Distributable(world, imageID, x, y, right, 2)
{
    setVisible(true);
    m_maxTicks = max((unsigned int)100, 300 - 10 * getWorld()->getLevel());
}

Spawnable::~Spawnable()
{}

void Spawnable::spawnableDoSomething(int score)
{
    if (isAnnoyed())
        return;
    
    incTicks();
    
    if (getTicks() >= m_maxTicks)
    {
        setAnnoyed();
        return;
    }
    
    if (getWorld()->collidedwithTM(getX(), getY(), 3.5))
    {
        setAnnoyed();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(score);
        getWorld()->getTM()->addToInventory(getID());
        return;
    }
    
}

Sonar::Sonar(StudentWorld* world)
      :Spawnable(world, TID_SONAR, 0, MAX_Y)
{}

Sonar::~Sonar()
{}

void Sonar::doSomething()
{
    spawnableDoSomething(75);
}

Water::Water(StudentWorld* world, int x, int y)
      :Spawnable(world, TID_WATER_POOL, x, y)
{}

Water::~Water()
{}

void Water::doSomething()
{
    spawnableDoSomething(100);
}
