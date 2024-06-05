#include "StudentWorld.h"
#include "Actor.h"
#include <string>
#include <sstream>
#include <cmath>
#include <queue>
#include <map>
using namespace std;

StudentWorld::StudentWorld(string assetDir)
    : GameWorld(assetDir)
{
    m_tm = nullptr;
    m_totalTicks = 0;
    m_numBarrels = 0;
    m_numProtesters = 0;
    m_targetProtesters = 0;
    m_ticksPerProtester = 0;
    m_probSpawnable = 0;
    m_probHardcore = 0;
}

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

StudentWorld::~StudentWorld()
{}

int StudentWorld::init()
{
    //reset values
    m_totalTicks = 0;
    m_numProtesters = 0;
    m_targetProtesters = min(15, (int)(2 + getLevel() * 1.5));
    m_ticksPerProtester = max((unsigned int)25, 200 - getLevel());
    m_probSpawnable = getLevel() * 25 + 300;
    m_probHardcore = min((unsigned int)90, getLevel() * 10 + 30);
    
    for (int row = 0; row < VIEW_HEIGHT; row++)                                         //populate m_earth with Earth objects
        for (int col = 0; col < VIEW_WIDTH; col++)
            m_earth[col][row] = new Earth(this, col, row);
    
    for (int row = 0; row < VIEW_HEIGHT - 4; row++)                                     //make Earth objects visible
        for (int col = 0; col < VIEW_WIDTH; col++)
            if (col < 30 || col > 33 || row < 4)
                m_earth[col][row]->setVisible(true);
    
                                                                                        //populate m_actors w/ distributables
    int B = min(getLevel()/ 2 + 2, (unsigned int)9);                                    //boulder
    for (int i = 0; i < B; i++)
        addDistributable('B');
    long int size = m_actors.size();
    for (int i = 0; i < size; i++)                                                      //remove the earth behind boulders
        for (int row = m_actors[i]->getY(); row < m_actors[i]->getY() + 4; row++)
            for (int col = m_actors[i]->getX(); col < m_actors[i]->getX() + 4; col++)
                m_earth[col][row]->setVisible(false);
    
    int G = max(5 - getLevel() / 2, (unsigned int)2);                                   //gold
    for (int i = 0; i < G; i++)
        addDistributable('G');
    m_numBarrels = min(2 + getLevel(), (unsigned int)21);                               //barrel
    for (int i = 0; i < m_numBarrels; i++)
        addDistributable('L');
    
    m_tm = new TunnelMan(this);                                                         //initialize tunnel man
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    setDisplayText();
    m_tm->doSomething();
    vector<Actor*>::iterator it = m_actors.begin();
    while (it != m_actors.end())                        //allow all objects in m_actors to do something
    {
        if (!(*it)->isAnnoyed())
        {
            (*it)->doSomething();
            
            if (m_tm->isAnnoyed())
            {
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
            
            if (m_numBarrels == 0)
            {
                playSound(SOUND_FINISHED_LEVEL);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
        it++;
    }
    
    addProtester();
    addSpawnable();
    removeDeadObjects();
    m_totalTicks++;                 //used to deal with spawning protesters and spawnables
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    //delete all dynamically allocated objects
    delete m_tm;
    for (int row = 0; row < VIEW_HEIGHT - 4; row++)
        for (int col = 0; col < VIEW_WIDTH; col++)
            delete m_earth[col][row];
    
    long int size = m_actors.size();
    
    for (long int i = size - 1; i >= 0; i--)
    {
        delete m_actors[i];
        m_actors.pop_back();
    }
}

TunnelMan* StudentWorld::getTM()
{
    return m_tm;
}

bool StudentWorld::isDiggable(int x, int y, Actor::Direction d, bool isTM)
{
    bool containsEarth = false;
    
    if (d == Actor::left)
    {
        for (int row = y; row <= y + 3; row++)
        {
            if (m_earth[x][row]->isVisible())
            {
                containsEarth = true;
                if(isTM)
                    m_earth[x][row]->setVisible(false);
            }
        }
    }
    else if (d == Actor::right)
    {
        for (int row = y; row <= y + 3; row++)
        {
            if (m_earth[x + 3][row]->isVisible())                   //TM coordinate is bottom left, so add 3 to x for direction up
            {
                containsEarth = true;
                if(isTM)
                    m_earth[x + 3][row]->setVisible(false);
            }
        }
    }
    else if (d == Actor::up)
    {
        for (int col = x; col <= x + 3; col++)
        {
            if (m_earth[col][y + 3]->isVisible())                   //TM coordinate is bottom left, so add 3 to y for direction up
            {
                containsEarth = true;
                if(isTM)
                    m_earth[col][y + 3]->setVisible(false);
            }
        }
    }
    else if (d == Actor::down)
    {
        for (int col = x; col <= x + 3; col++)
        {
            if (m_earth[col][y]->isVisible())
            {
                containsEarth = true;
                if(isTM)
                    m_earth[col][y]->setVisible(false);
            }
        }
    }
    
    return containsEarth;
}

bool StudentWorld::nextToBoulder(int x, int y, Actor::Direction d)
{
    
    long int size = m_actors.size();
    for (int i = 0; i < size; i++)
    {
                                                                            // !(...) allows for the boulder to not consider itself
                                                                            // if it considered itself, it would immediately die after the earth below was removed
                                                                            // since it would technically be next to a boulder based on this function
        if (m_actors[i]->getID() == TID_BOULDER
            && !(x == m_actors[i]->getX() && y == m_actors[i]->getY()))
        {
                                                                            //add +2 to simualte center
                                                                            //center is automatically updated based on direction
            if (d == Actor::left)
            {
                if (getDistance(m_actors[i]->getX() + 2, m_actors[i]->getY() + 2,
                                x + 1, y + 2) < 3.2)
                    return true;
            }
            else if (d == Actor::right)
            {
                if (getDistance(m_actors[i]->getX() + 2, m_actors[i]->getY() + 2,
                                x + 3, y + 2) < 3.2)
                    return true;
            }
            else if (d == Actor::up)
            {
                if (getDistance(m_actors[i]->getX() + 2, m_actors[i]->getY() + 2,
                                x + 2, y + 3) < 3.2)
                    return true;
            }
            else if (d == Actor::down)
            {
                if (getDistance(m_actors[i]->getX() + 2, m_actors[i]->getY() + 2,
                                x + 2, y + 1) < 3.2)
                    return true;
            }
        }
    }
    
//    long int size = m_actors.size();
//    for (int i = 0; i < size; i++)
//    {
//        if (m_actors[i]->getID() == TID_BOULDER)
//        {
//            if (d == Actor::left)
//            {
//                if (x - 1 == m_actors[i]->getX() + 3)
//                    if (y >= m_actors[i]->getY() - 3 && y <= m_actors[i]->getY() + 3)
//                        return true;
//            }
//            else if (d == Actor::right)
//            {
//                //x + 4 since using left bottom corner as coordinate, and right checks right bottom corner
//                if (x + 4 == m_actors[i]->getX())
//                    if (y >= m_actors[i]->getY() - 3 && y <= m_actors[i]->getY() + 3)
//                        return true;
//            }
//            else if (d == Actor::up)
//            {
//                if (y + 4 == m_actors[i]->getY())
//                    if (x >= m_actors[i]->getX() - 3 && x <= m_actors[i]->getX() + 3)
//                        return true;
//            }
//            else if (d == Actor::down)
//            {
//                if (y - 1 == m_actors[i]->getY() + 3)
//                    if (x >= m_actors[i]->getX() - 3 && x <= m_actors[i]->getX() + 3)
//                        return true;
//            }
//        }
//    }
    
    return false;
}

bool StudentWorld::collidedwithTM(int x, int y, double distance)
{
    //add 2 to x and y to get coords of center
    if (getDistance(m_tm->getX() + 2, m_tm->getY() + 2, x + 2, y + 2) < distance)
        return true;
    return false;
}

Protester* StudentWorld::collidedwithProtester(int x, int y)
{
    long int size = m_actors.size();
    for (int i = 0; i < size; i++)
    {
        //check if the item in m_actors is a protester and if a collision occured
        if ((m_actors[i]->getID() == TID_PROTESTER || m_actors[i]->getID() == TID_HARD_CORE_PROTESTER)
            && (getDistance(m_actors[i]->getX() + 2, m_actors[i]->getY() + 2, x + 2, y + 2) < 3.5))
            return (Protester*)m_actors[i];
    }
    return nullptr;
}

void StudentWorld::decBarrels()
{
    m_numBarrels--;
}

void StudentWorld::decProtesters()
{
    m_numProtesters--;
}

void StudentWorld::addActor(Actor* a)
{
    m_actors.push_back(a);
}

//Actor::Direction StudentWorld::findDirectiontoTM(int x, int y)
//{
//    int tmY = m_tm->getY();
//    int tmX = m_tm->getX();
//    if (y == tmY)
//    {
//        if (x > tmX)                                                //case left
//        {
//            for (int i = x; i > tmX; i--)
//                if (m_earth[i][y]->isVisible())
//                    return Actor::none;
//            return Actor::left;
//        }
//        else if (x < tmX)                                           //case right
//        {
//            for (int i = x; i < tmX; i++)
//                if (m_earth[i][y]->isVisible())
//                    return Actor::none;
//            return Actor::right;
//        }
//    }
//    else if (x == tmX)
//    {
//        if (y < tmY)                                                //case up
//        {
//            for (int i = y; i < tmY; i++)
//                if (m_earth[x][i]->isVisible())
//                    return Actor::none;
//            return Actor::up;
//        }
//        else if (y > tmY)                                           //case down
//        {
//            for (int i = y; i > tmY; i--)
//                if (m_earth[x][i]->isVisible())
//                    return Actor::none;
//            return Actor::down;
//        }
//    }
//    return Actor::none;
//}

Actor::Direction StudentWorld::findDirectiontoTM(int x, int y)
{
    int tmY = m_tm->getY();
    int tmX = m_tm->getX();
                                                                    //the tunnel man can only be in the line of sight of the protester if either the X or Y is equal
                                                                    //case: only y is equal
    if (y == tmY)
    {
                                                                    //If the protester has a greater x than the tunnel man, it is to the right
        if (x > tmX)                                                //case face left
        {
                                                                    //in any case where earth/boulder blocks the view (for each 4 pixels of the height), don't change direction
            for (int row = y; row < y + 4; row++)
                for (int col = x; col > tmX; col--)
                    if (m_earth[col][row]->isVisible()
                        || nextToBoulder(col, row, Actor::left))
                        return Actor::none;
            return Actor::left;                                     //if at this point, there is no blockage, so return a direction
        }
                                                                    //If the protester has a lesser x, it is to the left
        else if (x < tmX)                                           //case face right
        {
            for (int row = y; row < y + 4; row++)
                for (int col = x; col < tmX; col++)
                    if (m_earth[col][row]->isVisible()
                        || nextToBoulder(col, row, Actor::right))
                        return Actor::none;
            return Actor::right;
        }
    }
    else if (x == tmX)                                              //case: only x is equal
    {
                                                                    //if y < tmY, protester is below
        if (y < tmY)                                                //case face up
        {
            for (int col = x; col < x + 4; col++)
                for (int row = y; row < tmY; row++)
                    if (m_earth[col][row]->isVisible()
                        || nextToBoulder(col, row, Actor::up))
                        return Actor::none;
            return Actor::up;
        }
                                                                    //if y > tmY, protester is above
        else if (y > tmY)                                           //case face down
        {
            for (int col = x; col < x + 4; col++)
                for (int row = y; row > tmY; row--)
                    if (m_earth[col][row]->isVisible()
                        || nextToBoulder(col, row, Actor::down))
                        return Actor::none;
            return Actor::down;
        }
    }
    return Actor::none;
}

double StudentWorld::getDistance(int x1, int y1, int x2, int y2)
{
    double diffX = x1 - x2;
    double diffY = y1 - y2;
    double total = sqrt(pow(diffX, 2) + pow(diffY, 2));
    return total;
}

void StudentWorld::makeDistributablesVisible(double distance)
{
    long int size = m_actors.size();
    for (int i = 0; i < size; i++)
    {
        if (m_actors[i]->isDistributable() &&             //if the object is a distributable and within distance of the tunnel man, make visible
            getDistance(m_tm->getX(), m_tm->getY(), m_actors[i]->getX(), m_actors[i]->getY()) < distance)
                m_actors[i]->setVisible(true);
    }
}

bool StudentWorld::validProtesterMove(int x, int y, Actor::Direction d)
{
    if (x > MAX_X || y > MAX_Y || x < 0 || y < 0)                               //protester cannot move out of bounds
        return false;
    if (d == Actor::left)                                                       //make sure there is no earth or boulder next to it
    {
        if (!isDiggable(x, y, Actor::left, false) && !nextToBoulder(x, y, d))
            return true;
    }
    else if (d == Actor::right)
    {
        if (!isDiggable(x, y, Actor::right, false) && !nextToBoulder(x, y, d))
            return true;
    }
    else if (d == Actor::up)
    {
        if (!isDiggable(x, y, Actor::up, false) && !nextToBoulder(x, y, d))
            return true;
    }
    else if (d == Actor::down)
    {
        if (!isDiggable(x, y, Actor::down, false) && !nextToBoulder(x, y, d))
            return true;
    }
    return false;
}

bool operator<(const StudentWorld::m_coordinate& lhs, const StudentWorld::m_coordinate& rhs)
{
    if (rhs.m_x != lhs.m_x)
        return rhs.m_x < lhs.m_x;
    return rhs.m_y < lhs.m_y;
}

void StudentWorld::moveTowardsExit(Protester* p)
{
    for (int col = 0; col <= MAX_X; col++)
        for (int row = 0; row <= MAX_Y; row++)
            m_visited[col][row] = false;
    
    map<m_coordinate, m_coordinate> parent;                         //map to keep track of last move
    queue<m_coordinate> q;
    q.push(m_coordinate(60, 60));
    m_visited[60][60] = true;
    int colChanges[] = {0, 0, -1, 1};
    int rowChanges[] = {-1, 1, 0, 0};
    
    while (!q.empty())
    {
        m_coordinate curr = q.front();
        q.pop();
        if (curr.m_x == p->getX() && curr.m_y == p->getY())
            break;
        for (int i = 0; i < 4; i++)
        {
            Actor::Direction d;
            int col = curr.m_x + colChanges[i];
            int row = curr.m_y + rowChanges[i];
            
            if (i == 0)                                         //this is based on col changes and row changes array
                d = Actor::down;
            else if (i == 1)
                d = Actor::up;
            else if (i == 2)
                d = Actor::left;
            else
                d = Actor::right;
            
            if (validProtesterMove(col, row, d) && !m_visited[col][row])
            {
                m_visited[col][row] = true;
                m_coordinate nextMove(col, row);
                parent[nextMove] = curr;                        //track the parent of the next move
                q.push(nextMove);
            }
        }
    }
    
    m_coordinate trace(p->getX(), p->getY());
    m_coordinate dir = parent[trace];
    
//    map<m_coordinate, m_coordinate>::iterator it = parent.begin();
//    while (it != parent.end())
//    {
//        cout << "(" << it->first.m_x << ", " << it->first.m_y << ")";
//        cout << "(" << it->second.m_x << ", " << it->second.m_y << ")";
//        cout << endl;
//        it++;
//    }
//    cout << "instance!\n";

    if (dir.m_x > p->getX())                                        //move based on the coordinate of the parent
        p->directionalMovement(Actor::right);
    else if (dir.m_x < p->getX())
        p->directionalMovement(Actor::left);
    else if (dir.m_y > p->getY())
        p->directionalMovement(Actor::up);
    else if (dir.m_y < p->getY())
        p->directionalMovement(Actor::down);
}

void StudentWorld::sensePlayer(Protester* p)
{
    for (int col = 0; col <= MAX_X; col++)
        for (int row = 0; row <= MAX_Y; row++)
            m_visited[col][row] = false;
    
    map<m_coordinate, m_coordinate> parent;
    queue<m_coordinate> q;
    q.push(m_coordinate(m_tm->getX(), m_tm->getY()));               //similar to exit, but the target is the tunnel man, not (60, 60)
    m_visited[m_tm->getX()][m_tm->getY()] = true;
    int colChanges[] = {0, 0, -1, 1};
    int rowChanges[] = {-1, 1, 0, 0};
    
    while (!q.empty())
    {
        m_coordinate curr = q.front();
        q.pop();

        for (int i = 0; i < 4; i++)
        {
            Actor::Direction d;
            int col = curr.m_x + colChanges[i];
            int row = curr.m_y + rowChanges[i];
            if (i == 0)
                d = Actor::down;
            else if (i == 1)
                d = Actor::up;
            else if (i == 2)
                d = Actor::left;
            else
                d = Actor::right;
            if (validProtesterMove(col, row, d) && !m_visited[col][row])
            {
                m_visited[col][row] = true;
                m_coordinate nextMove(col, row);
                parent[nextMove] = curr;
                q.push(nextMove);
            }
        }
    }
    
    m_coordinate trace(p->getX(), p->getY());
    m_coordinate dir = parent[trace];

    if (dir.m_x > p->getX())
        p->directionalMovement(Actor::right);
    else if (dir.m_x < p->getX())
        p->directionalMovement(Actor::left);
    else if (dir.m_y > p->getY())
        p->directionalMovement(Actor::up);
    else if (dir.m_y < p->getY())
        p->directionalMovement(Actor::down);
}

//PRIVATE ---------------------------------------------------------------------------------------------------------------------------------------

void StudentWorld::removeDeadObjects()
{
    vector<Actor*>::iterator it = m_actors.begin();
    while (it != m_actors.end())
    {
        if ((*it)->isAnnoyed())
        {
            delete (*it);
            it = m_actors.erase(it);
        }
        else
            it++;
    }
}

bool StudentWorld::checkWaterCoords(int x, int y)
{
    for (int col = x; col < x + 4; col++)
        for (int row = y; row < y + 4; row++)
            if (m_earth[col][row]->isVisible())
                return false;
    return true;
}

void StudentWorld::spawnWater()
{
    int x = rand() % 61;
    int y = rand() % 61;
    while (!checkWaterCoords(x, y))         //can spawn anywhere where there is not earth
    {
        x = rand() % 61;
        y = rand() % 61;
    }
    m_actors.push_back(new Water(this, x, y));
}

void StudentWorld::addDistributable(char c)
{
    //make sure do not spawn at edges, so have the + 1
    int x = rand() % 59 + 1;
    //check if in the middle
    while (x >= 26 && x <= 34)
        x = rand() % 59 + 1;
    int y;
    if (c == 'B')
    {
        y = rand() % 35 + 1;
        y += 20;                                //make sure boulder is not too low
    }
    else
        y = rand() % 55 + 1;
    
    while (!checkDistance(x, y))
    {
        x = rand() % 59 + 1;
        while (x >= 26 && x <= 34)
            x = rand() % 59 + 1;
        if (c == 'B')
        {
            y = rand() % 35 + 1;
            y += 20;
        }
        else
            y = rand() % 55 + 1;
    }
    
    if (c == 'B')
        m_actors.push_back(new Boulder(this, x, y));
    else if (c == 'G')
        m_actors.push_back(new Gold(this, x, y, true));
    else if (c == 'L')
        m_actors.push_back(new Barrel(this, x, y));
}

void StudentWorld::addProtester()
{
    if (m_totalTicks % m_ticksPerProtester == 0                     //check if its a valid time to add a protester
        && m_numProtesters < m_targetProtesters)
    {
        if (rand() % 101 <= m_probHardcore && m_totalTicks != 0)    //roll for hardcore spawn, first protester is always regular
            m_actors.push_back(new HardcoreProtester(this));
        else
            m_actors.push_back(new RegularProtester(this));
        m_numProtesters++;
    }
    else
        return;
}

void StudentWorld::addSpawnable()
{
    if (rand() % m_probSpawnable == 0)
    {
        if (rand() % 5 == 0)                                        //20% chance of sonar spawning
            m_actors.push_back(new Sonar(this));
        else
            spawnWater();
    }
}

bool StudentWorld::checkDistance(int x, int y)
{
    long int size = m_actors.size();
    for (int i = 0; i < size; i++)
    {
        if (m_actors[i]->isDistributable())
        {
            if (getDistance(x, y, m_actors[i]->getX(), m_actors[i]->getY()) < 6)
                return false;
        }
    }
    return true;
}

void StudentWorld::setDisplayText()
{
    string s = formatDisplayText(getLevel(), getLives(), m_tm->getHP(), m_tm->getSquirts(),
                                 m_tm->getGold(), m_numBarrels, m_tm->getSonar(), getScore());
    setGameStatText(s);
}

string StudentWorld::formatDisplayText(int level, int lives, int health, int squirts,
                                       int gold, int numBarrels, int sonar, int score)
{
    ostringstream scoress;
    ostringstream oss;
    scoress << "Scr: " << scoress.fill('0') << setw(6) << score;
    oss << scoress.str()
        << " Lvl: " << setw(2) << level
        << " Lives: " << setw(1) << lives
        << " Hlth: " << setw(3) << health * 10
        << "% Wtr: " << setw(2) << squirts
        << " Gld: " << setw(2) << gold
        << " Sonar: " << setw(2) << sonar
        << " Oil Left: " << setw(2) << numBarrels;
    return oss.str();
}
