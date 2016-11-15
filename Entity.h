#include "SheetSprite.h"

#define FIXED_TIMESTEP 0.0166666f
#define TILE_SIZE 0.03f

class Entity {
public:
	Entity();

	float rotation;

	bool alive = true;

	SheetSprite sprite;

	void draw(ShaderProgram);

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
};