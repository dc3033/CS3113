#pragma once
#include "ShaderProgram.h"

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size, float x, float y);

	void Draw(ShaderProgram program);

	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
	float reverseAspect = height / width;
	float x;
	float y;
};