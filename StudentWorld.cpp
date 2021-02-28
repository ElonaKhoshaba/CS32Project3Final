#include "StudentWorld.h"
#include "Actor.h"
#include "GameConstants.h"
#include <string>
#include <iostream> // defines the overloads of the << operator
#include <sstream>  // defines the type std::ostringstream
using namespace std;


///////////////////////////////////////////////////////////////////////////
// Main Game Control Implementations (start, continue, end)
///////////////////////////////////////////////////////////////////////////
GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}


// Sets StudentWorld's data members to default values
StudentWorld::StudentWorld(string assetPath)
    : GameWorld(assetPath)
{
    m_ghostRacer = nullptr;
    m_lastYCord = 0;
    m_bonusPoints = 0;
    m_souls2save = 0;
}


// Initializes current level: creates GhostRacer, borders, bonus points, souls
int StudentWorld::init()
{
    m_ghostRacer = new GhostRacer(this);
    initializeBorders();
    m_bonusPoints = BONUS_POINTS;
    m_souls2save = 2 * getLevel() + 5;
    return GWSTATUS_CONTINUE_GAME;
}

// Executes actions of level each tick (20 times per second)
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
        // Allow each actor to do something if they're alive
        if (! (*it)->isDead())
        {
            (*it)->doSomething();
        }

        // If GhostRacer is dead, immediately end level to game over or restart
        if (m_ghostRacer->getHealth() <= 0 || m_ghostRacer->isDead())
        {
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }

        // Level completed successfully
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

    // Potentially add new actors to the game 
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


// Deletes all remaining actors (including GhostRacer)
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


///////////////////////////////////////////////////////////////////////////
// Init() Helper Functions
///////////////////////////////////////////////////////////////////////////

// Adds initial borders on screen before level begins
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


///////////////////////////////////////////////////////////////////////////
// Move() Helper Functions
///////////////////////////////////////////////////////////////////////////

// Formats the stats displayed on the top of each level
string StudentWorld::formatDisplayText()
{
    ostringstream displayText;
    displayText << "Score: " << getScore() << "  Lvl: " << getLevel() << "  Souls2Save: " << m_souls2save
        << "  Lives: " << getLives() << "  Health: " << m_ghostRacer->getHealth() <<
        "  Sprays: " << m_ghostRacer->getNumSprays() << "  Bonus: " << m_bonusPoints;
    string display = displayText.str();
    return display;
}

///////////////////////////
// Add New Actor Methods //
///////////////////////////

// Adds new borderlines if some have gone off the screen
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

// Calculates chance of adding a new actor to the level
bool StudentWorld::chanceOf(int left, int right) const
{
    int chance = max(left, right);
    int rand = randInt(0, chance - 1);
    return rand == 0;
}

// Attempts to add new human pedestrians based on chance
void StudentWorld::addNewHumanPeds()
{
    if (chanceOf(200 - getLevel() * 10, 30))
        m_actors.push_back(new HumanPedestrian(this, randInt(0, VIEW_WIDTH), VIEW_HEIGHT));
}

// Attempts to add new zombie pedestrians based on chance
void StudentWorld::addNewZombiePeds()
{
    if (chanceOf(100 - getLevel() * 10, 20))
        m_actors.push_back(new ZombiePedestrian(this, randInt(0, VIEW_WIDTH), VIEW_HEIGHT));
}

// Attempts to add new zombie cabs based on chance
void StudentWorld::addNewZombieCabs()
{
    if (chanceOf(100 - getLevel() * 10, 20))
        attemptToAddZombieCab();
}

// Attempts to add new oil slicks based on chance
void StudentWorld::addNewOilSlicks()
{
    if (chanceOf(150 - getLevel() * 10, 40))
        m_actors.push_back(new OilSlick(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
}

// Attempts to add new holy water refills based on chance
void StudentWorld::addNewHolyWaterRefillGoodies()
{
    if (chanceOf(100 + 10 * getLevel(), 0))
        m_actors.push_back(new HolyWaterGoodie(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
}

// Attempts to add new lost souls based on chance
void StudentWorld::addNewLostSoulGoodies()
{
    if(chanceOf(100, 0))
        m_actors.push_back(new SoulGoodie(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
}


/////////////////////////////////
// Zombie Cab Helper functions //       
/////////////////////////////////

// Attempts to add a new zombie cab based on the presence of collision avoidance worthy actors 
// in the lane
void StudentWorld::attemptToAddZombieCab()
{
    int cur_lane = randInt(0, 2); // Pick random candidiate lane, corresponds to position in array
    double lanes[3] = { LEFT_CENTER, ROAD_CENTER, RIGHT_CENTER };
    int numLanes;
    double startX, startY, initialYVel;
    // For up to each of the three lanes, check if there is no actor too close to the bottom of the screen
    // to add a zombie cab, or if that doesn't work, if there is no actor too close to the top of the screen
    // to add a zombie cab
    for (numLanes = 1; numLanes <= 3; numLanes++)
    {
        double min = getClosestAbove(lanes[cur_lane], 0);
        if (min == 999 || (min != 999 && min > VIEW_HEIGHT / 3.0))  // cur lane is the chosen lane
        {
            startX = lanes[cur_lane];
            startY = SPRITE_HEIGHT / 2.0;
            initialYVel = m_ghostRacer->getYVelocity() + randInt(2, 4);
            break;
        }

        double max = getClosestBelow(lanes[cur_lane], VIEW_HEIGHT - 1);
        if (max == -999 || (max != -999 && max < (VIEW_HEIGHT * 2.0 / 3.0)))
        {
            startX = lanes[cur_lane];
            startY = VIEW_HEIGHT - SPRITE_HEIGHT / 2.0;
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

// Returns the y coordinate of the closest actor ABOVE the reference x and y coordinates
// If no such actor exists, returns 999
// Check INCLUDES GhostRacer
int StudentWorld::getClosestAbove(double refX, double refY)
{
    vector<double> possibleYCords;
    bool foundActorInLane = false;
    int curLane = determineLaneNumber(refX);
    // GhostRacer included
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

    // Find the minimum of all eligble y coordinates
    double min = 999;
    for (vector<double>::iterator it = possibleYCords.begin(); it != possibleYCords.end(); it++)
    {
        if ((*it) < min)
        {
            min = (*it);
        }
    }

    if (foundActorInLane)
        return min;
    return 999;
}

// Returns the y coordinate of the closest actor BELOW the reference x and y coordinates
// If no such actor exists, returns -999
// Check EXCLUDES GhostRacer
int StudentWorld::getClosestBelow(double refX, double refY)
{
    vector<double> possibleYCords;
    bool foundActorInLane = false;
    int curLane = determineLaneNumber(refX);

    // Gather all possible y coordinates in the lane of refX that are LESS than refY
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

    // Find max of all eligible y coordinates
    double max = -999;
    for (vector<double>::iterator it = possibleYCords.begin(); it != possibleYCords.end(); it++)
    {
        if ((*it) > max)
        {
            max = (*it);
        }
    }

    if (foundActorInLane)
        return max;
    return -999;
}

// Returns true if the given x coordinate x1, is within the left and right bounds
// Assumes leftBoundary and rightBoundary are valid lane values
bool StudentWorld::isWithinLane(double x1, double leftBoundary, double rightBoundary) const
{
    if (x1 >= leftBoundary && x1 < rightBoundary)
        return true;
    return false;
}

// Determines lane number of the given x coordinate
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

///////////////////////////////////////////////////////////////////////////
// Public Actor Interaction Methods
///////////////////////////////////////////////////////////////////////////

// Determines if two actors overlap
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




