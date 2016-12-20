#include "Entity.h"

Entity::Entity(){
	velocityX = 0.0f;
	velocityY = 0.0f;
	accelerationX = 0.0f;
	accelerationY = 0.0f;
};

void Entity::draw(ShaderProgram program) {
	unsigned int textureID = sprite.textureID;
	float u = sprite.u;
	float v = sprite.v;
	float sWidth = sprite.width;
	float sHeight = sprite.height;
	float aspect = sWidth / sHeight;

	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLfloat texCoords[] = {
		u, v + sHeight,
		u + sWidth, v,
		u, v,
		u + sWidth, v,
		u, v + sHeight,
		u + sWidth, v + sHeight
	};

	float vertices[] = {
		-size * aspect, -size,
		size * aspect, size,
		-size * aspect, size,
		size * aspect, size,
		-size * aspect, -size,
		size * aspect, -size
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}

void Entity::resize(float newSize){
	size = newSize;
	height = size * 2;
	width = size * 2;
}

bool Entity::collideEntity(Entity* other){
	float thisTop = y + height / 2.0f;
	float thisBottom = y - height / 2.0f;
	float thisLeft = x - width / 2.0f;
	float thisRight = x + width / 2.0f;
	float otherTop = other->y + other->height / 2.0f;
	float otherBottom = other->y - other->height / 2.0f;
	float otherLeft = other->x - other->width / 2.0f;
	float otherRight = other->x + other->width / 2.0f;

	if (thisBottom > otherTop || thisTop < otherBottom || thisLeft > otherRight || thisRight < otherLeft)
		return false;
	else
		return true;
}