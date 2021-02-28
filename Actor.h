#ifndef ACTOR_INCLUDED
#define ACTOR_INCLUDED
#include "GraphObject.h"
#include "StudentWorld.h"
using namespace std;

class StudentWorld;

///////////////////////////////////////////////////////////////////////////
// Actor Class Declaration
///////////////////////////////////////////////////////////////////////////

class Actor : public GraphObject
{
public:
    Actor(StudentWorld* sw, int imageID, double x, double y, double size = 2.0, int dir = 0, int depth = 2)
        : GraphObject(imageID, x, y, dir, size, depth), m_world(sw), m_alive(true), m_yVel(-4) {}
    virtual ~Actor() {}

    // Action to perform for each tick.
    virtual void doSomething();

    // Is this actor dead?
    bool isDead() const { return m_alive == false; }

    // Mark this actor as dead.
    void setDead() { m_alive = false; }

    // Get this actor's world
    StudentWorld* getWorld() const { return m_world; }

    // Get this actor's vertical speed.
    double getYVelocity() const { return m_yVel; }

    // Get this actor's horizontal speed (most actors don't have an x velocity)
    virtual double getXVelocity() const { return 0; }
   
    // Set this actor's vertical speed.
    void setYVelocity(double yVel) { m_yVel = yVel; }

    // If this actor is affected by holy water projectiles, then inflict that
    // affect on it and return true; otherwise, return false.
    virtual bool beSprayedIfAppropriate() { return false; }

    // Does this object affect zombie cab placement and speed?
    virtual bool isCollisionAvoidanceWorthy() const { return false; }

    // Adjust the x coordinate by dx to move to a position with a y coordinate
    // determined by this actor's vertical speed relative to GhostRacser's
    // vertical speed.  Return true if the new position is within the view;
    // otherwise, return false, with the actor dead.
    virtual bool moveRelativeToGhostRacerVerticalSpeed(double dx); //moveTo(getX() + dx, relative speed) // dx is getXVelocity()

private:
    StudentWorld* m_world;  // Pointer to this actor's student world
    bool m_alive;           // Tracks alive status of actor
    double m_yVel;          // Vertical velocity of actor

    // Specialized doSomething functions for derived classes (do nothing by default)
    virtual void doSomethingSpecializedA() { return; }
    virtual void doSomethingSpecializedB() { return; }

protected:
    static const int FACING_STRAIGHT = 90;		// 90 Degrees
    static const int FACING_HORIZONTAL = 180;	// 180 Degrees
    static const int FACING_DOWN = 270;         // 270 degrees
    const double NUM_PI = atan(1) * 4;			// pi = 3.14

    // converts given degrees to radians and returns result in radians
    double degreesToRad(double deg) { return deg * (NUM_PI / 180); }
};


///////////////////////////////////////////////////////////////////////////
// Borderline Class Declaration
///////////////////////////////////////////////////////////////////////////

class BorderLine : public Actor
{
public:
    BorderLine(StudentWorld* sw, double x, double y, bool isYellow)
        : Actor(sw, isYellow ? IID_YELLOW_BORDER_LINE : IID_WHITE_BORDER_LINE, x, y) { }
    virtual ~BorderLine() {}
    // Borderlines do nothing other than move down vertically, which is what the Actor class
    // do something function does, so no other functions are needed
};


///////////////////////////////////////////////////////////////////////////
// Agent Class Declaration (includes GhostRacer, Pedestrians, ZombieCabs)
///////////////////////////////////////////////////////////////////////////

class Agent : public Actor
{
public:
    Agent(StudentWorld* sw, int imageID, double x, double y, double size, int dir, int hp)
        : Actor(sw, imageID, x, y, size, dir, 0), m_health(hp) {}
    virtual ~Agent() {}

    // All agents impact zombie cab placement
    virtual bool isCollisionAvoidanceWorthy() const { return true; }

    // Get hit points.
    int getHealth() const { return m_health; }

    // Increase hit points by hp. This is virtual because GhostRacer's max health is 100
    virtual void setHealth(int hp) { m_health = hp; }

    // Do what the spec says happens when hp units of damage is inflicted.
    // Return true if this agent dies as a result, otherwise false.
    virtual bool takeDamageAndPossiblyDie(int hp);

    // What sound should play when this agent is damaged but does not die?
    virtual int soundWhenHurt() const = 0;

    // What sound should play when this agent is damaged and dies?
    virtual int soundWhenDie() const = 0;

private:
    int m_health; // Tracks agent HP
    // Specialized damage functions for agents
    virtual void specializedAgentDamageA() { return; }
    virtual void specializedAgentDamageB() { getWorld()->playSound(soundWhenHurt()); }
};


///////////////////////////////////////////////////////////////////////////
// GhostRacer Class Declaration
///////////////////////////////////////////////////////////////////////////

class GhostRacer : public Agent
{
public:
    GhostRacer(StudentWorld* sw, double x = 128, double y = 32)
        : Agent(sw, IID_GHOST_RACER, x, y, 4.0, FACING_STRAIGHT, 100), m_sprays(10)
    {
        setYVelocity(0);    // GhostRacer's intial y velocity is 0, not -4
    }
    virtual ~GhostRacer() {}

    virtual int soundWhenHurt() const { return SOUND_VEHICLE_CRASH; }
    
    virtual int soundWhenDie() const { return SOUND_PLAYER_DIE; }

    // Overrides Agent::sethealth(int hp) because GhostRacer's cap health is 100
    virtual void setHealth(int hp) { Agent::setHealth(hp); if (getHealth() > 100) Agent::setHealth(100); }

    // How many holy water projectiles does the object have?
    int getNumSprays() const { return m_sprays; }

    // Increase the number of holy water projectiles the object has.
    void increaseSprays(int amt) { m_sprays += amt; }

    // Spin as a result of hitting an oil slick.
    void spin();

    // GhostRacer moves differently than other Actors
    virtual bool moveRelativeToGhostRacerVerticalSpeed(double dx);

private:
    int m_sprays;                       // Number of sprays GhostRacer has
    const int INCREMENT_DIR = 8;		// Degrees to increment GhostRacer by when moving left/right
    const int HP_LOSS_HIT_EDGE = -10;	// How much HP Ghost Racer loses for hitting a road edge
    const int MAX_RACER_VEL = 5;		// GhostRacer's maximum Y velocity
    const int MIN_RACER_VEL = -1;		// GhostRacer's mimimum Y velocity
    
    // What happens when GhostRacer hits left edge, right edge, and player hits keys
    virtual void doSomethingSpecializedA();
    // Overrides Agent class, which plays sound when hurt by default
    virtual void specializedAgentDamageB() { return; }

};


///////////////////////////////////////////////////////////////////////////
// NonPlayableCharacter Class Declaration (includes pedestrains and cabs)
///////////////////////////////////////////////////////////////////////////

class NonPlayableCharacter : public Agent   // NPCs all have movement plans and horizontal speeds, all can get sprayed
{
public:
    NonPlayableCharacter(StudentWorld* sw, int imageID, double x, double y, double size, int dir, int hp)
        : Agent(sw, imageID, x, y, size, dir, hp), m_xVel(0), m_movePlan(0) {}
    virtual ~NonPlayableCharacter() {}

    // Overrides Actor::getXVelocity() const which returns 0
    virtual double getXVelocity() const { return m_xVel; }

    // Sets x velocity of NPC
    void setXVelocity(double s) { m_xVel = s; }

    // Gets movement plan
    int getMovePlan() const { return m_movePlan; }
    
    // Sets movement plan
    void setMovePlan(int movePlan) { m_movePlan = movePlan; }

    // Movement plan for each NPC varies
    virtual void pickMovePlan() = 0;

    // By default NPCs are impacted by sprays and
    // take one hit point of damage when sprayed
    virtual bool beSprayedIfAppropriate() { takeDamageAndPossiblyDie(-1); return true; }

private:
    double m_xVel;  // Tracks horizontal speed of NPC
    int m_movePlan; // Tracks movement plan of NPC
};


///////////////////////////////////////////////////////////////////////////
// Pedestrian Class Declaration (includes humans and zombies)
///////////////////////////////////////////////////////////////////////////

class Pedestrian : public NonPlayableCharacter
{
public:
    Pedestrian(StudentWorld* sw, int imageID, double x, double y, double size)
        : NonPlayableCharacter(sw, imageID, x, y, size, 0, 2) {}
    virtual ~Pedestrian() {}

    virtual int soundWhenHurt() const { return SOUND_PED_HURT; }
   
    virtual int soundWhenDie() const { return SOUND_PED_DIE; }

    // Picks the movement plan of the pedestrian (same for all pedestrians)
    virtual void pickMovePlan();
};


///////////////////////////////////////////////////////////////////////////
// HumanPedestrian Class Declaration 
///////////////////////////////////////////////////////////////////////////

class HumanPedestrian : public Pedestrian
{
public:
    HumanPedestrian(StudentWorld* sw, double x, double y)
        : Pedestrian(sw, IID_HUMAN_PED, x, y, 2.0) {}
    virtual ~HumanPedestrian() {}

    // Overrides NPC::beSprayedIfAppropiate() because humans don't take
    // damage with holy water, instead they change their direction and
    // play a sound
    virtual bool beSprayedIfAppropriate();
    
    // Human pedestrians don't "take damage"
    virtual bool takeDamageAndPossiblyDie(int hp) { return false; }

private:
    // If GhostRacer overlaps with human, player immediately loses a life
    virtual void doSomethingSpecializedA();
    
    // Human picks new movement plan after moving
    virtual void doSomethingSpecializedB();
};

///////////////////////////////////////////////////////////////////////////
// ZombiePedestrian Class Declaration 
///////////////////////////////////////////////////////////////////////////

class ZombiePedestrian : public Pedestrian
{
public:
    ZombiePedestrian(StudentWorld* sw, double x, double y)
        : Pedestrian(sw, IID_ZOMBIE_PED, x, y, 3.0), m_ticksToNextGrunt(0) {}
    virtual ~ZombiePedestrian() {}

private:
    int m_ticksToNextGrunt;                 // Tracks tick to next grunt
    
    // If zombie hits GhostRacer, zombie dies and player takes 5 points of damage
    // Zombie pedestrian faces downwards if close enough to GhostRacer
    virtual void doSomethingSpecializedA();
    
    // Zombie pedestrian picks new movement plan
    virtual void doSomethingSpecializedB();
    
    // When a zombie pedestrian dies by holy water projectiles, the player 
    // gets points and it might drop a healing goodie in its place
    virtual void specializedAgentDamageA();

};


///////////////////////////////////////////////////////////////////////////
// ZombiePedestrian Class Declaration 
///////////////////////////////////////////////////////////////////////////

class ZombieCab : public NonPlayableCharacter
{
public:
    ZombieCab(StudentWorld* sw, double x, double y)
        : NonPlayableCharacter(sw, IID_ZOMBIE_CAB, x, y, 4.0, FACING_STRAIGHT, 3), m_hasDamagedGhostRacer(false) {}
    virtual ~ZombieCab() {}
    
    virtual int soundWhenHurt() const { return SOUND_VEHICLE_CRASH; }
   
    virtual int soundWhenDie() const { return SOUND_VEHICLE_DIE; }
    
    // Zombie cab changes velocity with movement plan
    virtual void pickMovePlan();

private:
    bool m_hasDamagedGhostRacer;

    // If zombie cab collides with GhostRacer, it damages the player and flies off the screen 
    virtual void doSomethingSpecializedA();
    
    // After moving, if the closest collision avoidance worthy actor above/below the zombie cab 
    // is within 96 pixels, the cab changes speed
    virtual void doSomethingSpecializedB();
    
    // If zombie cab is killed by holy water, then it might leave an
    // oil slick and the player gets 200 points
    virtual void specializedAgentDamageA();
    
    const int LEFT_DIRECTION = 120;     // 120 degrees
    const int RIGHT_DIRECTION = 60;     // 60 degrees
};


///////////////////////////////////////////////////////////////////////////
// Spray Class Declaration 
///////////////////////////////////////////////////////////////////////////

class Spray : public Actor
{
public:
    Spray(StudentWorld* sw, double x, double y, int dir)
        : Actor(sw, IID_HOLY_WATER_PROJECTILE, x, y, 1.0, dir, 1), m_maxTravelDistance(160) {}
    virtual ~Spray() {}

    // Every tick, the spray does something to the first actor impacted by holy water sprays 
    virtual void doSomething(); 

private:
    int m_maxTravelDistance;    // Tracks the distance left the spray can travel before dissapating
};


///////////////////////////////////////////////////////////////////////////
// GhostRacerActivatedObject Class Declaration 
///////////////////////////////////////////////////////////////////////////

class GhostRacerActivatedObject : public Actor
{
public:
    GhostRacerActivatedObject(StudentWorld* sw, int imageID, double x, double y, double size, int dir = 0)
        : Actor(sw, imageID, x, y, size, dir) {}
    virtual ~GhostRacerActivatedObject() {}

    // Do the object's special activity (increase health, spin Ghostracer, etc.)
    virtual void doActivity(GhostRacer* gr) = 0;
    
    // Return the object's increase to the score when activated.
    virtual int getScoreIncrease() const = 0;
    
    // Return the sound to be played when the object is activated. Defaults to goodie sound
    virtual int getSound() const { return SOUND_GOT_GOODIE; }

protected:
    // All activated objects play a sound, increase player score (might be by zero)
    // and do something when GhostRacer overlaps with them
    virtual void doSomethingSpecializedB();
};


///////////////////////////////////////////////////////////////////////////
// OilSlick Class Declaration 
///////////////////////////////////////////////////////////////////////////

class OilSlick : public GhostRacerActivatedObject
{
public:
    OilSlick(StudentWorld* sw, double x, double y)
        : GhostRacerActivatedObject(sw, IID_OIL_SLICK, x, y, randInt(2, 5)) {}
    virtual ~OilSlick() {}

    // GhostRacer spins when it overlaps with an oil slick
    virtual void doActivity(GhostRacer* gr);
    
    // No points are awarded to the player when they hit an oil slick
    virtual int getScoreIncrease() const { return 0; }
    
    // Sound when GhostRacer overlaps with oil slick
    virtual int getSound() const { return SOUND_OIL_SLICK; }
};

class HealingGoodie : public GhostRacerActivatedObject
{
public:
    HealingGoodie(StudentWorld* sw, double x, double y)
        : GhostRacerActivatedObject(sw, IID_HEAL_GOODIE, x, y, 1.0) {}
    virtual ~HealingGoodie() {}

    // GhostRacer gets 10 hp when it overlaps with a healing goodie
    virtual void doActivity(GhostRacer* gr);

    // Player gets 250 points for getting a healing goodie
    virtual int getScoreIncrease() const { return 250; }

    // Healing goodies are destroyed when sprayed
    virtual bool beSprayedIfAppropriate() { setDead(); return true; }
};



class HolyWaterGoodie : public GhostRacerActivatedObject
{
public:
    HolyWaterGoodie(StudentWorld* sw, double x, double y) // direction facing straight
        : GhostRacerActivatedObject(sw, IID_HOLY_WATER_GOODIE, x, y, 2.0, FACING_STRAIGHT) {}
    virtual ~HolyWaterGoodie() {}

    // GhostRacer gets 10 sprays when it overlaps with holywater refill
    virtual void doActivity(GhostRacer* gr);
    
    // Player gets 50 points for getting a holy water refill
    virtual int getScoreIncrease() const { return 50; }
    
    // Holy water goodies are destroyed when sprayed
    virtual bool beSprayedIfAppropriate() { setDead(); return true; }
};

class SoulGoodie : public GhostRacerActivatedObject
{
public:
    SoulGoodie(StudentWorld* sw, double x, double y)
        : GhostRacerActivatedObject(sw, IID_SOUL_GOODIE, x, y, 4.0) {}
    virtual ~SoulGoodie() {}
   
    // When GhostRacer overlaps with a soul, the number of souls left to save 
    // in the current level decrements
    virtual void doActivity(GhostRacer* gr);
    
    // Player gets 100 points for getting a soul
    virtual int getScoreIncrease() const { return 100; }
    
    // Soul goodies have their own sound, orrides general goodie sound
    virtual int getSound() const { return SOUND_GOT_SOUL; }

private:
    // Every tick, the soul goodie spins by 10 degrees clockwise after it moves down
    virtual void doSomethingSpecializedB();
};

#endif // ACTOR_INCLUDED