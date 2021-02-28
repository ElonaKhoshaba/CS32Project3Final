#ifndef STUDENTWORLD_INCLUDED
#define STUDENTWORLD_INCLUDED

#include "GameWorld.h"
#include <string>
#include <list>
using namespace std;

///////////////////////////////////////////////////////////////////////////
// Game Constants
///////////////////////////////////////////////////////////////////////////
const double LANE_WIDTH = ROAD_WIDTH / 3;

const double LEFT_CENTER = ROAD_CENTER - LANE_WIDTH;       // Center of left lane
const double RIGHT_CENTER = ROAD_CENTER + LANE_WIDTH;      // Center of right lane

const double LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH / 2;
const double RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH / 2;

const int LEFT_LANE = 1;
const int MIDDLE_LANE = 2;
const int RIGHT_LANE = 3;

const int BORDER_SPEED = -4;
const int BONUS_POINTS = 5000;

///////////////////////////////////////////////////////////////////////////
// Student World 
///////////////////////////////////////////////////////////////////////////
class Actor;
class GhostRacer;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);

    virtual ~StudentWorld();

    // Initializes current level
    virtual int init();

    // Executes actions of level each tick (20 times per second)
    virtual int move();

    // Deletes all remaining actors (including GhostRacer)
    virtual void cleanUp();

    //////////////////////////////////////
    // Public Actor Interaction Methods //
    //////////////////////////////////////

    // Return a pointer to the world's GhostRacer
    GhostRacer* getRacer() const { return m_ghostRacer; }

    // Return true if actor a1 overlaps actor a2, otherwise false.
    bool overlaps(const Actor* a1, const Actor* a2) const;

    // If actor a overlaps this world's GhostRacer, return a pointer to the
    // GhostRacer; otherwise, return nullptr
    GhostRacer* getOverlappingGhostRacer(Actor* a) const;

    // Add an actor to the world
    void addActor(Actor* a) { m_actors.push_back(a); }

    // Record that a soul was saved
    void recordSoulSaved() { m_souls2save--; }

    // If actor a overlaps some live actor that is affected by a holy water
    // projectile, inflict a holy water spray on that actor and return true;
    // otherwise, return false
    bool sprayFirstAppropriateActor(Actor* a);

    // Returns the y coordinate of the closest actor ABOVE the reference x and y coordinates
    // If no such actor exists, returns 999
    // Check INCLUDES GhostRacer
    int getClosestAbove(double refX, double refY);

    // Returns the y coordinate of the closest actor BELOW the reference x and y coordinates
    // If no such actor exists, returns -999
    // Check EXCLUDES GhostRacer
    int getClosestBelow(double refX, double refY);

private:
    list<Actor*> m_actors;      // Container that stores all actors in game (except for GhostRacer)
    GhostRacer* m_ghostRacer;   // Pointer to this world's GhostRacer
    double m_lastYCord;         // Y Coordinate of the last white borderline added 
    int m_bonusPoints;          // Bonus points in current level   
    int m_souls2save;           // Number of souls to save before level ends

    // Doesn't allow bonus points to reach a negative value
    void decreaseBonusPoints() { m_bonusPoints--; if (m_bonusPoints < 0) { m_bonusPoints = 0; } }

    // Calculates chance of adding a new actor to the level
    bool chanceOf(int left, int right) const;

    // Determines which lane the given x coordinate is in (assumes left and right boundary
    // are valid lane parameters)
    bool isWithinLane(double x1, double leftBoundary, double rightBoundary) const;

    // Determines lane number of given x coordinate
    int determineLaneNumber(double x1) const;

    // Formats the stats displayed on the top of each level
    string formatDisplayText();

    ///////////////////////////
    // Add New Actor Methods //
    ///////////////////////////
    void initializeBorders();
    void addNewBorderLines();
    void addNewHumanPeds();
    void addNewZombiePeds();
    void addNewZombieCabs();
    void addNewOilSlicks();
    void addNewHolyWaterRefillGoodies();
    void addNewLostSoulGoodies();
    // Attempts to add a new zombie cab based on the presence of collision avoidance 
    // worthy actors in the lane
    void attemptToAddZombieCab();
};

#endif // STUDENTWORLD_INCLUDED