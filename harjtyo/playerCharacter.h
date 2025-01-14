#ifndef PLAYERCHARACTERH
#define PLAYERCHARACTERH

#include <SFML\Graphics.hpp>
#include "bullet.h"

enum Direction {
	SOUTH, SOUTHEAST, EAST, NORTHEAST,
	NORTH, NORTHWEST, WEST, SOUTHWEST
};

class PlayerCharacter : public sf::Sprite {
public:
	PlayerCharacter(const sf::Texture&);
	void updateTimer();
	void animate();
	void setDirection(int);
	sf::Vector2f getSize();
	bool isReadyToFire();
	void setReadyToFire(bool);
	bool checkHit(const sf::Vector2f&);
	bool isAlive();
	void setHitbox(sf::FloatRect);
	int getHealthPoints();
private:
	sf::Vector2f size;
	sf::Clock cooldownTimer;
	float cooldown;
	bool readyToFire = false;
	int direction = 0;
	int frameNumber = 0;
	int healthPoints;
	bool alive;
	sf::FloatRect hitbox;
};

#endif