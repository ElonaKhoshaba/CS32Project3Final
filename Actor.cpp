#include "Actor.h"
#include "StudentWorld.h"



// Adjust the x coordinate by dx to move to a position with a y coordinate
// determined by this actor's vertical speed relative to GhostRacer's
// vertical speed.  Return true if the new position is within the view;
// otherwise, return false, with the actor dead.
bool Actor::moveRelativeToGhostRacerVerticalSpeed(double dx) //moveTo(getX() + dx, relative speed) // dx is getXVelocity()
{
	double vert_speed = getYVelocity() - getWorld()->getRacer()->getYVelocity();
	double horiz_speed = dx;
	double new_y = getY() + vert_speed;
	double new_x = getX() + horiz_speed;
	moveTo(new_x, new_y);
	// Actor has gone off the screen, so set alive status to false
	if (getY() < 0 || getX() < 0 || getX() > VIEW_WIDTH || getY() > VIEW_HEIGHT)
	{
		setDead();
		return false;
	}
	return true;
}

void Actor::doSomething()
{
	// Actor is destroyed, do nothing
	if (isDead())
	{
		return;
	}
	doSomethingSpecializedA();
	moveRelativeToGhostRacerVerticalSpeed(getXVelocity());
	doSomethingSpecializedB();
}



// Agent Class
// Do what the spec says happens when hp units of damage is inflicted.
// Return true if this agent dies as a result, otherwise false.
bool Agent::takeDamageAndPossiblyDie(int hp)
{
	setHealth(getHealth() + hp);
	if (getHealth() <= 0)
	{
		setDead();
		getWorld()->playSound(soundWhenDie());
		specializedAgentDamageA();
		return true;

	}
	specializedAgentDamageB(); // setDirection(direction);
	// specializedDamageC();
	return false;
	
}

// Ghost Racer Class
void GhostRacer::doSomethingSpecializedA()
{
	int ch;
	// If GhostRacer hits left road edge
	if (getX() <= LEFT_EDGE)
	{
		if (getDirection() > FACING_STRAIGHT)
		{
			takeDamageAndPossiblyDie(HP_LOSS_HIT_EDGE);
			setDirection(FACING_STRAIGHT - INCREMENT_DIR);
			getWorld()->playSound(soundWhenHurt());
		}	
	}

	// If GhostRacer hits right road edge
	else if (getX() >= RIGHT_EDGE)
	{
		if (getDirection() < FACING_STRAIGHT)
		{
			takeDamageAndPossiblyDie(HP_LOSS_HIT_EDGE);
			setDirection(FACING_STRAIGHT + INCREMENT_DIR);
			getWorld()->playSound(soundWhenHurt());

		}
			
	}

	// If user hits a key
	else if (getWorld()->getKey(ch))
	{
		switch (ch)
		{
		case KEY_PRESS_SPACE:
			if (getNumSprays() >= 1)
			{
				double dir = getDirection();
				double delta_x = SPRITE_HEIGHT * cos(degreesToRad(dir)) + getX();
				double delta_y = SPRITE_HEIGHT * sin(degreesToRad(dir)) + getY();
				getWorld()->addActor(new Spray(getWorld(), delta_x, delta_y, dir));
				getWorld()->playSound(SOUND_PLAYER_SPRAY);
				m_sprays--;
			}
			break;
		case KEY_PRESS_LEFT:
			if (getDirection() < FACING_STRAIGHT + 3 * INCREMENT_DIR)
				setDirection(getDirection() + INCREMENT_DIR);
			break;
		case KEY_PRESS_RIGHT:
			if (getDirection() > FACING_STRAIGHT - 3 * INCREMENT_DIR)
				setDirection(getDirection() - INCREMENT_DIR);
			break;
		case KEY_PRESS_UP:
			if (getYVelocity() < MAX_RACER_VEL)
				setYVelocity(getYVelocity() + 1);
			break;
		case KEY_PRESS_DOWN:
			if (getYVelocity() > MIN_RACER_VEL)
				setYVelocity(getYVelocity() - 1);
			break;
		}
	}
}

bool GhostRacer::moveRelativeToGhostRacerVerticalSpeed(double dx)
{
	double max_shift_per_tick = 4.0;
	double direction = degreesToRad(getDirection());
	double delta_x = cos(direction) * max_shift_per_tick;
	double cur_x = getX();
	double cur_y = getY();
	moveTo(cur_x + delta_x, cur_y);
	return true;
}

void GhostRacer::spin()
{
	//  Ghost Racer can be spun around(by driving over an oil slick).If an oil slick tries
	//	to spin Ghost Racer, Ghost Racer must adjust its direction by a random integer
	//	between[5, 20] degrees clockwise or counterclockwise of its current direction
	int changeDir = randInt(5, 20);
	int negOrPos = randInt(-1, 0) == -1 ? -1 : 1;
	int newDirection = getDirection() + changeDir * negOrPos;
	if (newDirection > 120)
		setDirection(120);
	else if (newDirection < 60)
		setDirection(60);
	else 
		setDirection(newDirection);
}

// Pedestrian Class
void Pedestrian::pickMovePlan()
{
	// Pick new movement plan
	const int nSpeeds = 6;
	const int possibleXVels[nSpeeds] = { -3, -2, -1, 1, 2, 3 };
	int randIndex = randInt(0, nSpeeds - 1);
	setXVelocity(possibleXVels[randIndex]);
	setMovePlan(randInt(4, 32));

	if (getXVelocity() < 0)
	{
		setDirection(180);
	}
	else if (getXVelocity() > 0)
	{
		setDirection(0);
	}
}


// Human Pedestrian Class
void HumanPedestrian::doSomethingSpecializedA()
{
	if (getWorld()->overlaps(this, getWorld()->getRacer()))
	{
		setDead();
		getWorld()->getRacer()->setDead();
		return;
	}
}
void HumanPedestrian::doSomethingSpecializedB()
{
	setMovePlan(getMovePlan() - 1);
	if (getMovePlan() > 0)
	{
		return;
	}
	pickMovePlan();
}

// If this actor is affected by holy water projectiles, then inflict that
// affect on it and return true; otherwise, return false.

bool HumanPedestrian::beSprayedIfAppropriate()
{
	// When damaged by holy water, human pedestrians reverse their direction
	setXVelocity(getXVelocity() * -1);
	setDirection(getDirection() == 180 ? 0 : 180);	// If direction is 180, set 0; if 0 set to 180
	getWorld()->playSound(soundWhenHurt());
	return true;
}

 // Zombie Pedestrian Class
void ZombiePedestrian::doSomethingSpecializedA()
{
	Agent* overlappingRacer = getWorld()->getOverlappingGhostRacer(this);
	if (overlappingRacer != nullptr)
	{
		overlappingRacer->takeDamageAndPossiblyDie(-5);
		this->takeDamageAndPossiblyDie(-2);
		return;
	}	
	
	// Zombie pedestrian x coordinate within 30 pixels of ghost racer and in front of ghostracer on road
	double distanceBetweenZombieAndRacer = getX() - getWorld()->getRacer()->getX();
	if (fabs(distanceBetweenZombieAndRacer) <= 30 && getY() > getWorld()->getRacer()->getY())
	{
		setDirection(FACING_DOWN);
		if (distanceBetweenZombieAndRacer == 0)
		{
			setXVelocity(0);
		}
		else
		{
			double dv = distanceBetweenZombieAndRacer < 0 ? 1 : -1;
			setXVelocity(dv);
		}
		m_ticksToNextGrunt--;
		if (m_ticksToNextGrunt <= 0)
		{
			getWorld()->playSound(SOUND_ZOMBIE_ATTACK);
			m_ticksToNextGrunt = 20;
		}
	}
}

void ZombiePedestrian::doSomethingSpecializedB()
{
	if (getMovePlan() > 0)
	{
		setMovePlan(getMovePlan() - 1);
		return;
	}
	pickMovePlan();
}

void ZombiePedestrian::specializedAgentDamageA()
{
	GhostRacer* overlappingRacer = getWorld()->getOverlappingGhostRacer(this);
	// If zombie ped does not currently overlap w/GhostRacer (ie, it didn't die due to GR colliding with it)
	// then there is a 1 in 5 chance that zombie ped will add a new healing goodie at its current position
	if (overlappingRacer == nullptr)
	{
		int chance = randInt(1, 5);
		if (chance == 1)
		{
			getWorld()->addActor(new HealingGoodie(getWorld(), getX(), getY()));
		}
	}
	// Player gets 150 points
	getWorld()->increaseScore(150);
}



// Zombie Cab Class-----------------------------------------------------
void ZombieCab::doSomethingSpecializedA()
{
	GhostRacer* overlappingRacer = getWorld()->getOverlappingGhostRacer(this);
	// If ZombieCab overlaps with Ghost Racer
	if (overlappingRacer != nullptr)
	{
		if (m_hasDamagedGhostRacer)
		{
			; // Do nothing
		}
		else
		{
			getWorld()->playSound(soundWhenHurt());
			// Do 20 points of damage to GhostRacer
			overlappingRacer->takeDamageAndPossiblyDie(-20);
			// Zombie Cab is to the left of GR
			if (getX() <= overlappingRacer->getX())
			{
				setXVelocity(-5);
				setDirection(LEFT_DIRECTION + randInt(0, 19)); // 120 degrees plus rand [0,20)
			}
			else
			{
				setXVelocity(5);
				setDirection(RIGHT_DIRECTION + randInt(0, 19));
			}
			m_hasDamagedGhostRacer = true;
		}
	}
}

void ZombieCab::doSomethingSpecializedB()
{
	if (getYVelocity() > getWorld()->getRacer()->getYVelocity())
	{
		// Find closest actor, INCLUDING GHOST RACER, that is above the zombie cab
		// Find the ninmum y value that we track
		if (getWorld()->getClosestAbove(getX(), getY()))
		{
			setYVelocity(getYVelocity() - 0.5);
			return;
		}
	}

	else
	{
		// Find closest actor, EXCLUDING GHOST RACER, that is below the zombie cab
		if (getWorld()->getClosestBelow(getX(), getY()))
		{
			setYVelocity(getYVelocity() + 0.5);
			return;
		}
	}

	setMovePlan(getMovePlan() - 1);
	if (getMovePlan() > 0)
	{
		return;
	}
	pickMovePlan();
}

void ZombieCab::pickMovePlan()
{
	setMovePlan(randInt(4, 32));
	setYVelocity(getYVelocity() + randInt(-2, 2));
}

void ZombieCab::specializedAgentDamageA()
{

	int chance = randInt(1, 5);
	if (chance == 1)
	{
		getWorld()->addActor(new OilSlick(getWorld(), getX(), getY()));
	}

	// Player gets 200 points
	getWorld()->increaseScore(200);
}

// Spray Class
void Spray::doSomething()
{
	if (isDead())
	{
		return;
	}

	if (getWorld()->sprayFirstAppropriateActor(this))
	{
		setDead();
		return;
	}

	moveForward(SPRITE_HEIGHT);
	m_maxTravelDistance -= SPRITE_HEIGHT;
	
	if (getY() < 0 || getX() < 0 || getX() > VIEW_WIDTH || getY() > VIEW_HEIGHT)
	{
		setDead();
		return;
	}

	if (m_maxTravelDistance <= 0)
	{
		setDead();
	}
}

//if (isDead())
//{
//	return;
//}
//doSomethingSpecializedA();
//moveRelativeToGhostRacerVerticalSpeed(getXVelocity());
//doSomethingSpecializedB();

//double vert_speed = getYVelocity() - getWorld()->getRacer()->getYVelocity();
//double horiz_speed = dx;
//double new_y = getY() + vert_speed;
//double new_x = getX() + horiz_speed;
//moveTo(new_x, new_y);
//// Actor has gone off the screen, so set alive status to false
//if (getY() < 0 || getX() < 0 || getX() > VIEW_WIDTH || getY() > VIEW_HEIGHT)
//{
//	setDead();
//	return false;
//}
//return true;

// GhostRacerActivatedObject Class
void GhostRacerActivatedObject::doSomethingSpecializedB()
{
	GhostRacer* overlappingRacer = getWorld()->getOverlappingGhostRacer(this);
	if (overlappingRacer != nullptr)
	{
		doActivity(overlappingRacer);
		getWorld()->playSound(getSound());
		getWorld()->increaseScore(getScoreIncrease());
	}
}

// Oil Slick Class
void OilSlick::doActivity(GhostRacer* gr)
{
	gr->spin();
}

// Healing Goodie Class
void HealingGoodie::doActivity(GhostRacer* gr)
{
	gr->setHealth(gr->getHealth() + 10);
	setDead();
}

// Holy Water Goodie Class
void HolyWaterGoodie::doActivity(GhostRacer* gr)
{
	gr->increaseSprays(10);
	setDead();
}

// Soul Goodie Class
void SoulGoodie::doActivity(GhostRacer* gr)
{
	getWorld()->recordSoulSaved();
	setDead();
}

void SoulGoodie::doSomethingSpecializedB()
{
	GhostRacerActivatedObject::doSomethingSpecializedB();
	setDirection(getDirection() - 10);
}