#include "SheetSprite.h"

SheetSprite::SheetSprite(){};
SheetSprite::SheetSprite(unsigned int textureID, int index, int spriteCountX, int spriteCountY)
	:textureID(textureID), index(index), spriteCountX(spriteCountX), spriteCountY(spriteCountY){
	u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	width = 1.0f / (float)spriteCountX;
	height = 1.0f / (float)spriteCountY;
}