#pragma once
#include "Vector3.h"
#include "SheetSprite.h"

class Entity {
public:
	Entity();

	Vector3 position;
	Vector3 velocity;
	Vector3 size;

	float rotation;

	bool alive = 1;

	SheetSprite sprite;
};