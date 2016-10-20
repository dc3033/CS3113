#include "SheetSprite.h"

SheetSprite::SheetSprite(){};
SheetSprite::SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size, float x, float y)
: textureID(textureID), u(u), v(v), width(width), height(height), size(size), x(x), y(y) {};

void SheetSprite::Draw(ShaderProgram program) {
	glBindTexture(GL_TEXTURE_2D, textureID);

	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};

	float vertices[] = {
		x, y,
		x + size, y + (size * reverseAspect),
		x, y + (size * reverseAspect),
		x + size, y + (size * reverseAspect),
		x, y,
		x + size, y
	};


	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}