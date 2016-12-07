#include <vector>
#include <algorithm>

#include "SheetSprite.h"

#define FIXED_TIMESTEP 0.0166666f
#define PI 3.14159265

struct Vector{
	float x;
	float y;
};

class Entity {
public:
	Entity();

	SheetSprite sprite;

	void draw(ShaderProgram);

	Vector getVector(float vX, float vY);

	std::vector<Vector> getVertices();

	float x;
	float y;
	float width;
	float height;
	float velocityX;
	float velocityY;
	float rotation;
	float scaleX;
	float scaleY;
	float accelerationX;
	float accelerationY;

};