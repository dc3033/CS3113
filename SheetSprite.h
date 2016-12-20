#pragma once
#include "ShaderProgram.h"

class SheetSprite {
public:
	
	SheetSprite(unsigned int textureID, int index, int spriteCountX, int spriteCountY);
	SheetSprite();

	unsigned int textureID;
	int index;
	int spriteCountX;
	int spriteCountY;
	float u;
	float v;
	float width;
	float height;
};