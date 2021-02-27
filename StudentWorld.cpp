#include "StudentWorld.h"
#include "Actor.h"
#include "GameConstants.h"
#include <string>

#include <iostream> // defines the overloads of the << operator
#include <sstream>  // defines the type std::ostringstream
#include <iomanip>  // defines the manipulator setw
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
    : GameWorld(assetPath)
{
    m_ghostRacer = nullptr;
    m_lastYCord = 0;
    m_bonusPoints = 0;
    m_souls2save = 0;
}

int StudentWorld::init()
{
    m_ghostRacer = new GhostRacer(this);
    initializeBorders();
    m_bonusPoints = BONUS_POINTS;
    m_souls2save = 2 * getLevel() + 5;
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    decreaseBonusPoints();              // Decrease bonus by each tick
    if (! m_ghostRacer->isDead())
    {
        m_ghostRacer->doSomething();
    }
    list<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {

        if (! (*it)->isDead())
        {
            (*it)->doSomething();
        }

        if (m_ghostRacer->getHealth() <= 0 || m_ghostRacer->isDead())
        {
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }

        if (m_souls2save <= 0)
        {
            increaseScore(m_bonusPoints);
            return GWSTATUS_FINISHED_LEVEL;
        }
    }

    // Remove newly-dead actors after each tick
    for (it = m_actors.begin(); it != m_actors.end();)
    {
        if ((*it)->isDead())
        {
            delete (*it);
            it = m_actors.erase(it);
        }
        else
        {
            it++;
        }
    }

    // Potentially add new actors to the game (e.g., oil slicks or goodies or border lines)
    addNewBorderLines();
    addNewHumanPeds();
    addNewZombiePeds();
    addNewZombieCabs();
    addNewOilSlicks();
    addNewHolyWaterRefillGoodies();
    addNewLostSoulGoodies();

    // Update the Game Status Line
    setGameStatText(formatDisplayText());

    // The player hasn’t completed the current level and hasn’t died, so
    // continue playing the current level
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    delete m_ghostRacer;
    list<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end();)
    {
        delete (*it);
        it = m_actors.erase(it);
    }
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}



// ------------------------------------ Helper Functions ------------------------------------ //
//void StudentWorld::addBorder(int x, int y, bool isYellow)
//{
//    m_actors.push_back(new BorderLine(this, x, y, isYellow));
//
//}

string StudentWorld::formatDisplayText()
{
    ostringstream displayText;
    displayText << "Score: " << getScore() << "  Lvl: " << getLevel() << "  Souls2Save: " << m_souls2save
        << "  Lives: " << getLives() << "  Health: " << m_ghostRacer->getHealth() <<
        "  Sprays: " << m_ghostRacer->getNumSprays() << "  Bonus: " << m_bonusPoints;
    string display = displayText.str();
    return display;
}

void StudentWorld::initializeBorders()
{
    const int NUM_YELLOW_BORDER = VIEW_HEIGHT / SPRITE_HEIGHT;
    const int NUM_WHITE_BORDER = VIEW_HEIGHT / (4 * SPRITE_HEIGHT);

    // Add NUM_YELLOW_BORDER yellow border line objects on left and right
    for (int j = 0; j < NUM_YELLOW_BORDER; j++)
    {
        // Left Yellow BorderLines
        m_actors.push_back(new BorderLine(this, LEFT_EDGE, j * SPRITE_HEIGHT, true));
        // Right Yellow BorderLines
        m_actors.push_back(new BorderLine(this, RIGHT_EDGE, j * SPRITE_HEIGHT, true));
    }

    // Add NUM_WHITE_BORDER white border line objects on left and right
    for (int j = 0; j < NUM_WHITE_BORDER; j++)
    {
        // Left White BorderLines
        m_actors.push_back(new BorderLine(this, LEFT_EDGE + LANE_WIDTH, j * (4 * SPRITE_HEIGHT), false));
        // Right White BorderLines
        m_actors.push_back(new BorderLine(this, RIGHT_EDGE - LANE_WIDTH, j * (4 * SPRITE_HEIGHT), false));
    }
    // Saves y coordinate of last white border line added 
    m_lastYCord = (NUM_WHITE_BORDER - 1) * (4 * SPRITE_HEIGHT);
}

// ------------------------------------------------------------------------------------------------- //
// ------------------------------------ Move() HELPER FUNCTIONS ------------------------------------ //
// ------------------------------------------------------------------------------------------------- //

bool StudentWorld::chanceOf(int left, int right) const
{
    int chance = max(left, right);
    int rand = randInt(0, chance - 1);
    return rand == 0;
}

// ------------------------------------ Add New Actors Methods ------------------------------------ //
void StudentWorld::addNewBorderLines()
{
    m_lastYCord += BORDER_SPEED - m_ghostRacer->getYVelocity();
    double new_border_y = VIEW_HEIGHT - SPRITE_HEIGHT;
    double delta_y = new_border_y - m_lastYCord;

    if (delta_y >= SPRITE_HEIGHT)
    {
        // Add left and right yellow border lines
        m_actors.push_back(new BorderLine(this, LEFT_EDGE, new_border_y, true));
        m_actors.push_back(new BorderLine(this, RIGHT_EDGE, new_border_y, true));

    }
    if (delta_y >= (4 * SPRITE_HEIGHT))
    {
        // Add left and right white border lines
        m_actors.push_back(new BorderLine(this, LEFT_EDGE + LANE_WIDTH, new_border_y, false));
        m_actors.push_back(new BorderLine(this, RIGHT_EDGE - LANE_WIDTH, new_border_y, false));
        m_lastYCord = new_border_y;
    }
}

void StudentWorld::addNewHumanPeds()
{
    if (chanceOf(200 - getLevel() * 10, 30))
        m_actors.push_back(new HumanPedestrian(this, randInt(0, VIEW_WIDTH), VIEW_HEIGHT));
}

void StudentWorld::addNewZombiePeds()
{
    if (chanceOf(100 - getLevel() * 10, 20))
        m_actors.push_back(new ZombiePedestrian(this, randInt(0, VIEW_WIDTH), VIEW_HEIGHT));
}

void StudentWorld::addNewZombieCabs()
{
    if (chanceOf(100 - getLevel() * 10, 20))
        attemptToAddZombieCab();
}

void StudentWorld::addNewOilSlicks()
{
    if (chanceOf(150 - getLevel() * 10, 40))
        m_actors.push_back(new OilSlick(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
}

void StudentWorld::addNewHolyWaterRefillGoodies()
{
    if (chanceOf(100 + 10 * getLevel(), 0))
        m_actors.push_back(new HolyWaterGoodie(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
}
void StudentWorld::addNewLostSoulGoodies()
{
    if(chanceOf(100, 0))
        m_actors.push_back(new SoulGoodie(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
}

void StudentWorld::attemptToAddZombieCab()
{
    int cur_lane = randInt(0, 2); // Pick random candidiate lane, corresponds to position in array
    double lanes[3] = { LEFT_CENTER, ROAD_CENTER, RIGHT_CENTER };
    int numLanes;
    double startX, startY, initialYVel;
    for (numLanes = 1; numLanes <= 3; numLanes++)
    {
        Actor* closestActorToBottom = getClosestCollisionAvoidanceWorthyActorToBottomOfScreen(lanes[cur_lane]);
        if (closestActorToBottom == nullptr || closestActorToBottom->getY() > (VIEW_HEIGHT / 3))  // cur lane is the chosen lane
        {
            startX = lanes[cur_lane];
            startY = SPRITE_HEIGHT / 2;
            initialYVel = m_ghostRacer->getYVelocity() + randInt(2, 4);
            break;
        }

        Actor* closestActorToTop = getClosestCollisionAvoidanceWorthyActorToTopOfScreen(lanes[cur_lane]);
        if (closestActorToTop == nullptr || closestActorToTop->getY() < (VIEW_HEIGHT * 2 / 3))
        {
            startX = lanes[cur_lane];
            startY = VIEW_HEIGHT - SPRITE_HEIGHT / 2;
            initialYVel = m_ghostRacer->getYVelocity() - randInt(2, 4);
            break;
        }

        // Otherwise, current lane is too dangerous for adding a new cab, try a different lane
        if (cur_lane == 0)
            cur_lane = 1;
        else if (cur_lane == 1)
            cur_lane = 2;
        else
            cur_lane = 0;
    }
    // No viable zombie cabs found, so add none this tick
    if (numLanes > 3)
    {
        return;
    }

    Actor* newZombieCab = new ZombieCab(this, startX, startY);
    newZombieCab->setYVelocity(initialYVel);
    m_actors.push_back(newZombieCab);

}
bool StudentWorld::overlaps(const Actor* a1, const Actor* a2) const
{
    double delta_x = fabs(a1->getX() - a2->getX());
    double delta_y = fabs(a1->getY() - a2->getY());
    double radius_sum = a1->getRadius() + a2->getRadius();

    if (delta_x < (radius_sum * 0.25) && delta_y < (radius_sum * 0.6))
    {
        return true;	// The two actors overlap
    }

    return false;
}


// If actor a overlaps this world's GhostRacer, return a pointer to the
// GhostRacer; otherwise, return nullptr
GhostRacer* StudentWorld::getOverlappingGhostRacer(Actor* a) const
{
    if (overlaps(a, m_ghostRacer))
        return m_ghostRacer;
    return nullptr;
}

// If actor a overlaps some live actor that is affected by a holy water
   // projectile, inflict a holy water spray on that actor and return true;
   // otherwise, return false.  (See Actor::beSprayedIfAppropriate.)
bool StudentWorld::sprayFirstAppropriateActor(Actor* a)
{
    list<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->isDead())
            continue;
        if (overlaps(a, (*it)) && (*it)->beSprayedIfAppropriate())
        {
            return true;
        }
    }
    return false;

}

Actor* StudentWorld::getClosestCollisionAvoidanceWorthyActorToBottomOfScreen(double referencePointX)
{
    double minYValue = 999;
    Actor* closestActorToZero = nullptr;
    int currentLane = determineLaneNumber(referencePointX);
    if (currentLane == determineLaneNumber(m_ghostRacer->getX()))
    {
        minYValue = m_ghostRacer->getY();
        closestActorToZero = m_ghostRacer;
    }
    
    for (list<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->isCollisionAvoidanceWorthy())
        {
            if (currentLane == determineLaneNumber((*it)->getX()))
            {
                if ((*it)->getY() < minYValue)
                {
                    minYValue = (*it)->getY();
                    closestActorToZero = (*it);
                }
            }
        }
    }
    if (minYValue == 999)
        return nullptr;
    return closestActorToZero;
}

Actor* StudentWorld::getClosestCollisionAvoidanceWorthyActorToTopOfScreen(double referencePointX)
{
    double maxYValue = -999;
    Actor* closestActorToTop = nullptr;
    int currentLane = determineLaneNumber(referencePointX);
    if (currentLane == determineLaneNumber(m_ghostRacer->getX()))
    {
        maxYValue = m_ghostRacer->getY();
        closestActorToTop = m_ghostRacer;
    }

    for (list<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->isCollisionAvoidanceWorthy())
        {
            if (currentLane == determineLaneNumber((*it)->getX()))
            {
                if ((*it)->getY() > maxYValue)
                {
                    maxYValue = (*it)->getY();
                    closestActorToTop = (*it);
                }
            }
        }
    }
    if (maxYValue == -999)
        return nullptr;
    return closestActorToTop;
}

bool StudentWorld::getClosestAbove(double refX, double refY)
{
    vector<double> possibleYCords;
    bool foundActorInLane = false;
    int curLane = determineLaneNumber(refX);
    if (determineLaneNumber(m_ghostRacer->getX()) == curLane && m_ghostRacer->getY() > refY)
    {
        possibleYCords.push_back(m_ghostRacer->getY());
        foundActorInLane = true;
    }
    for (list<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->isCollisionAvoidanceWorthy())
        {
            if (determineLaneNumber((*it)->getX()) == curLane && (*it)->getY() > refY)
            {
                possibleYCords.push_back((*it)->getY());
                foundActorInLane = true;
            }
        }
    }

    double min = 999;
    for (vector<double>::iterator it = possibleYCords.begin(); it != possibleYCords.end(); it++)
    {
        if ((*it) < min)
        {
            min = (*it);
        }
    }

    if (foundActorInLane && fabs(min - refY) < 96)
        return true;
    return false;
}

bool StudentWorld::getClosestBelow(double refX, double refY)
{
    vector<double> possibleYCords;
    bool foundActorInLane = false;
    int curLane = determineLaneNumber(refX);
    for (list<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->isCollisionAvoidanceWorthy())
        {
            if (determineLaneNumber((*it)->getX()) == curLane && (*it)->getY() < refY)
            {
                possibleYCords.push_back((*it)->getY());
                foundActorInLane = true;
            }
        }
    }

    double min = 999;
    for (vector<double>::iterator it = possibleYCords.begin(); it != possibleYCords.end(); it++)
    {
        if ((*it) < min)
        {
            min = (*it);
        }
    }

    if (foundActorInLane && fabs(min - refY) < 96)
        return true;
    return false;
    
}

bool StudentWorld::isWithinLane(double x1, double leftBoundary, double rightBoundary) const
{
    if (x1 >= leftBoundary && x1 < rightBoundary)
        return true;
    return false;
}

int StudentWorld::determineLaneNumber(double x1) const
{
   
    if (isWithinLane(x1, LEFT_EDGE, LEFT_EDGE + LANE_WIDTH))
        return LEFT_LANE;

    if (isWithinLane(x1, LEFT_EDGE + LANE_WIDTH, RIGHT_EDGE - LANE_WIDTH))
        return MIDDLE_LANE;

    if (isWithinLane(x1, RIGHT_EDGE - LANE_WIDTH, RIGHT_EDGE))
        return RIGHT_LANE;

    return 999;
}

