#include "SheetSprite.h"

#define FIXED_TIMESTEP 0.0166666f

class Entity {
public:
	Entity();

	enum Direction{DIR_LEFT = -1, DIR_RIGHT = 1};

	int direction = DIR_RIGHT;

	bool alive = true;

	SheetSprite sprite;

	void draw(ShaderProgram);
	
	float size = 0.04;
	float height = size * 2;	
	float width = size * 2;

	void resize(float);

	float x;
	float y;
	float velocityX;
	float velocityY;
	float accelerationX;
	float accelerationY;

	bool collideLeft;
	bool collideRight;
	bool collideTop;
	bool collideBottom;
	bool collideEntity(Entity*);

	//Flag exclusive  members
	float p1CaptureStatus = 0.0;
	float p2CaptureStatus = 0.0;

	enum Status{ ST_NEUTRAL, ST_BLUE, ST_RED };

	int status = ST_NEUTRAL;

	//Bullet exclusive members
	enum BulletType{ B_NOT, B_SHOT, B_LASER, B_BOOM };

	int bType = B_NOT;

	//Explosion exclusive members
	enum BoomType{ BOOM_BOOM, BOOM_BOOM_BOOM};

	int boomType = BOOM_BOOM;

	float lifeTime;
};