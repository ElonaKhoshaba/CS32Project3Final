#ifndef STUDENTWORLD_INCLUDED
#define STUDENTWORLD_INCLUDED

#include "GameWorld.h"
#include <string>
#include <vector>
#include <list>
using namespace std;

// ------------------------------------ Constants ------------------------------------ //
const double LANE_WIDTH = ROAD_WIDTH / 3;

const double LEFT_CENTER = ROAD_CENTER - LANE_WIDTH;       // Center of left lane
const double RIGHT_CENTER = ROAD_CENTER + LANE_WIDTH;      // Center of right lane

const double LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH / 2;
const double RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH / 2;

const int BORDER_SPEED = -4;
const int BONUS_POINTS = 5000;

const int LEFT_LANE = 1;
const int MIDDLE_LANE = 2;
const int RIGHT_LANE = 3;

class Actor;
class GhostRacer;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();

    virtual int init();
    virtual int move();
    virtual void cleanUp();

    // Return a pointer to the world's GhostRacer.
    GhostRacer* getRacer() const { return m_ghostRacer; }

    // Return true if actor a1 overlaps actor a2, otherwise false.
    bool overlaps(const Actor* a1, const Actor* a2) const;

    // If actor a overlaps this world's GhostRacer, return a pointer to the
    // GhostRacer; otherwise, return nullptr
    GhostRacer* getOverlappingGhostRacer(Actor* a) const;

    // Add an actor to the world.
    void addActor(Actor* a) { m_actors.push_back(a); }

    // Record that a soul was saved.
    void recordSoulSaved() { m_souls2save--; }

    // If actor a overlaps some live actor that is affected by a holy water
    // projectile, inflict a holy water spray on that actor and return true;
    // otherwise, return false.  (See Actor::beSprayedIfAppropriate.)
    bool sprayFirstAppropriateActor(Actor* a);



    // TODO: Optimize
    Actor* getClosestCollisionAvoidanceWorthyActorToBottomOfScreen(double referencePointX);
    Actor* getClosestCollisionAvoidanceWorthyActorToTopOfScreen(double referencePointX);

    bool getClosestAbove(double refX, double refY);
    bool getClosestBelow(double refX, double refY);



private:
    list<Actor*> m_actors;

    GhostRacer* m_ghostRacer;
    double m_lastYCord;
    int m_bonusPoints;
    int m_souls2save;

    void decreaseBonusPoints() { m_bonusPoints--; if (m_bonusPoints < 0) { m_bonusPoints = 0; } }

    // Helper Functions
    bool chanceOf(int left, int right) const;   // calculates chance of adding new actor
    bool isWithinLane(double x1, double leftBoundary, double rightBoundary) const;
    int determineLaneNumber(double x1) const;
    string formatDisplayText();
    // Add New Actor Methods
    //void addBorder(int x, int y, bool isYellow);       // general pushback borderline 
    void initializeBorders();                   // for init()
    void addNewBorderLines();                   // for move()
    void addNewHumanPeds();
    void addNewZombiePeds();
    void addNewZombieCabs();
    void addNewOilSlicks();
    void attemptToAddZombieCab();
    void addNewHolyWaterRefillGoodies();
    void addNewLostSoulGoodies();
 
   
};

#endif // STUDENTWORLD_INCLUDED