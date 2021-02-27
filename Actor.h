#ifndef ACTOR_INCLUDED
#define ACTOR_INCLUDED

#include "GraphObject.h"
#include "StudentWorld.h"
using namespace std;

class StudentWorld;


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
       // virtual double changeInX() const { return 0; }
        virtual double getXVelocity() const { return 0; }

   
    private:
        StudentWorld* m_world;
        bool m_alive;
        double m_yVel;

        virtual void doSomethingSpecializedA() { return; }
        virtual void doSomethingSpecializedB() { return; }

    protected:
        static const int FACING_STRAIGHT = 90;		// 90 Degrees
        static const int FACING_HORIZONTAL = 180;	// 180 Degrees
        static const int FACING_DOWN = 270;         // 270 degrees
        const double NUM_PI = atan(1) * 4;			// pi = 3.14
        double degreesToRad(double deg) { return deg * (NUM_PI / 180); }

};

class BorderLine : public Actor
{
    public:
        BorderLine(StudentWorld* sw, double x, double y, bool isYellow)
            : Actor(sw, isYellow ? IID_YELLOW_BORDER_LINE : IID_WHITE_BORDER_LINE, x, y) { }
        virtual ~BorderLine() {}
};

class Agent : public Actor
{
    public:
        Agent(StudentWorld* sw, int imageID, double x, double y, double size, int dir, int hp)
            : Actor(sw, imageID, x, y, size, dir, 0), m_health(hp) {}
        virtual ~Agent() {}
       
        virtual bool isCollisionAvoidanceWorthy() const { return true; }

        // Get hit points.
        int getHealth() const { return m_health; }

        // Increase hit points by hp.
        virtual void setHealth(int hp) { m_health = hp; }

        // Do what the spec says happens when hp units of damage is inflicted.
        // Return true if this agent dies as a result, otherwise false.
        virtual bool takeDamageAndPossiblyDie(int hp);

        // What sound should play when this agent is damaged but does not die?
        virtual int soundWhenHurt() const = 0;

        // What sound should play when this agent is damaged and dies?
        virtual int soundWhenDie() const = 0;

    private:
        int m_health;
        virtual void specializedAgentDamageA() { return; }
        virtual void specializedAgentDamageB() { getWorld()->playSound(soundWhenHurt()); }
};

class GhostRacer : public Agent
{
    public:
        GhostRacer(StudentWorld* sw, double x = 128, double y = 32)
            : Agent(sw, IID_GHOST_RACER, x, y, 4.0, FACING_STRAIGHT, 100), m_sprays(10) 
        {
            setYVelocity(0);
        }

        virtual ~GhostRacer() {}
        
        //virtual void doSomething() {}
        virtual int soundWhenHurt() const { return SOUND_VEHICLE_CRASH; }
        virtual int soundWhenDie() const { return SOUND_PLAYER_DIE; }

        virtual void setHealth(int hp) {  Agent::setHealth(hp); if(getHealth() > 100) Agent::setHealth(100); }
        
        // How many holy water projectiles does the object have?
        int getNumSprays() const { return m_sprays; }

        // Increase the number of holy water projectiles the object has.
        void increaseSprays(int amt) { m_sprays += amt; }

        // Spin as a result of hitting an oil slick.
        void spin();
        virtual bool moveRelativeToGhostRacerVerticalSpeed(double dx);
       

    private:
        int m_sprays;
        const int INCREMENT_DIR = 8;		// Degrees to increment GhostRacer by when moving left/right
        const int HP_LOSS_HIT_EDGE = -10;	// How much HP Ghost Racer loses for hitting a road edge
        const int MAX_RACER_VEL = 5;		// GhostRacer's maximum Y velocity
        const int MIN_RACER_VEL = -1;		// GhostRacer's mimimum Y velocity
        virtual void doSomethingSpecializedA();
        virtual void specializedAgentDamageB() { return; }

};

// NPCs have moveplans and x velocities 
class NonPlayableCharacter : public Agent
{
    public:
        NonPlayableCharacter(StudentWorld* sw, int imageID, double x, double y, double size, int dir, int hp)
            : Agent(sw, imageID, x, y, size, dir, hp), m_xVel(0), m_movePlan(0) {}

        virtual double getXVelocity() const { return m_xVel; }

        void setXVelocity(double s) { m_xVel = s; }
        int getMovePlan() const { return m_movePlan; }
        void setMovePlan(int movePlan) { m_movePlan = movePlan; }

        virtual bool beSprayedIfAppropriate() { takeDamageAndPossiblyDie(-1); return true; }
        virtual void pickMovePlan() = 0;

    private:
        double m_xVel;
        int m_movePlan;
};

// Pedestrians have a health of 2
class Pedestrian : public NonPlayableCharacter
{
    public:
        Pedestrian(StudentWorld* sw, int imageID, double x, double y, double size)
            : NonPlayableCharacter(sw, imageID, x, y, size, 0, 2) {}
        virtual ~Pedestrian() {}

        virtual int soundWhenHurt() const { return SOUND_PED_HURT; }
        virtual int soundWhenDie() const { return SOUND_PED_DIE; }

        // Move the pedestrian. If the pedestrian doesn't go off screen and
        // should pick a new movement plan, pick a new plan.
        virtual void pickMovePlan();
};

// Health 2, Size 2.0
class HumanPedestrian : public Pedestrian
{
    public:
        HumanPedestrian(StudentWorld* sw, double x, double y)
            : Pedestrian(sw, IID_HUMAN_PED, x, y, 2.0) {}
        //virtual void doSomething() {}
        virtual bool beSprayedIfAppropriate();
        virtual bool takeDamageAndPossiblyDie(int hp) { return false; }
    private:
        virtual void doSomethingSpecializedA();
        virtual void doSomethingSpecializedB();
};

// Health 2, Size 3.0
class ZombiePedestrian : public Pedestrian
{
    public:
        ZombiePedestrian(StudentWorld* sw, double x, double y)
            : Pedestrian(sw, IID_ZOMBIE_PED, x, y, 3.0), m_ticksToNextGrunt(0){}
      //  virtual void doSomething();

    private:
        int m_ticksToNextGrunt;
        virtual void doSomethingSpecializedA();
        virtual void doSomethingSpecializedB();
        virtual void specializedAgentDamageA();

};

// Health 3, Size 4.0
class ZombieCab : public NonPlayableCharacter
{
    public:
        ZombieCab(StudentWorld* sw, double x, double y)
            : NonPlayableCharacter(sw, IID_ZOMBIE_CAB, x, y, 4.0, FACING_STRAIGHT, 3), m_hasDamagedGhostRacer(false) {} 
        // virtual void doSomething() { }
        virtual int soundWhenHurt() const { return SOUND_VEHICLE_CRASH; }
        virtual int soundWhenDie() const { return SOUND_VEHICLE_DIE; }
        virtual void pickMovePlan();
      
    private:
        bool m_hasDamagedGhostRacer;
        virtual void doSomethingSpecializedA();
        virtual void doSomethingSpecializedB();
        virtual void specializedAgentDamageA(); 
        const int LEFT_DIRECTION = 120;
        const int RIGHT_DIRECTION = 60; 
};


class Spray : public Actor
{
    public:
        Spray(StudentWorld* sw, double x, double y, int dir)
            : Actor(sw, IID_HOLY_WATER_PROJECTILE, x, y, 1.0, dir, 1), m_maxTravelDistance(160) {}
        virtual void doSomething();
    private:
        int m_maxTravelDistance;
};

class GhostRacerActivatedObject : public Actor
{
    public:
        GhostRacerActivatedObject(StudentWorld* sw, int imageID, double x, double y, double size, int dir = 0)
            : Actor(sw, imageID, x, y, size, dir) {}

        // Do the object's special activity (increase health, spin Ghostracer, etc.)
        virtual void doActivity(GhostRacer* gr) = 0;
        // Return the object's increase to the score when activated.
        virtual int getScoreIncrease() const = 0;
        // Return the sound to be played when the object is activated.
        virtual int getSound() const { return SOUND_GOT_GOODIE; }

    protected: 
        virtual void doSomethingSpecializedB();
};


class OilSlick : public GhostRacerActivatedObject
{
    public:
        OilSlick(StudentWorld* sw, double x, double y)
            : GhostRacerActivatedObject(sw, IID_OIL_SLICK, x, y, randInt(2, 5)) {}
        virtual void doActivity(GhostRacer* gr);
        virtual int getScoreIncrease() const { return 0; }
        virtual int getSound() const { return SOUND_OIL_SLICK; }
}; 

class HealingGoodie : public GhostRacerActivatedObject
{
    public:
        HealingGoodie(StudentWorld* sw, double x, double y)
            : GhostRacerActivatedObject(sw, IID_HEAL_GOODIE, x, y, 1.0) {}
        virtual void doActivity(GhostRacer* gr);
        virtual int getScoreIncrease() const { return 250; }
        virtual bool beSprayedIfAppropriate() { setDead(); return true; }
};



class HolyWaterGoodie : public GhostRacerActivatedObject
{
    public:
        HolyWaterGoodie(StudentWorld* sw, double x, double y) // direction facing straight
            : GhostRacerActivatedObject(sw, IID_HOLY_WATER_GOODIE, x, y, 2.0, FACING_STRAIGHT) {}
        virtual void doActivity(GhostRacer* gr);
        virtual int getScoreIncrease() const { return 50; }
        virtual bool beSprayedIfAppropriate() { setDead(); return true; }
};

class SoulGoodie : public GhostRacerActivatedObject
{
    public:
        SoulGoodie(StudentWorld* sw, double x, double y)
            : GhostRacerActivatedObject(sw, IID_SOUL_GOODIE, x, y, 4.0) {}
        virtual void doActivity(GhostRacer* gr);
        virtual int getScoreIncrease() const { return 100; }
        virtual int getSound() const { return SOUND_GOT_SOUL; }
    private:
        virtual void doSomethingSpecializedB();

};

#endif // ACTOR_INCLUDED