#include "Entity.h"

Entity::Entity(){
	velocityX = 0.0f;
	velocityY = 0.0f;
	accelerationX = 0.0f;
	accelerationY = 0.0f;
	scaleX = 1.0f;
	scaleY = 1.0f;
};

void Entity::draw(ShaderProgram program) {
	unsigned int textureID = sprite.textureID;

	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLfloat texCoords[] = {
		0, 1,
		1, 0,
		0, 0,
		1, 0,
		0, 1,
		1, 1
	};

	float vertices[] = {
		-width/2, -height/2,
		width/2, height/2,
		-width/2, height/2,
		width/2, height/2,
		-width/2, -height/2,
		width/2, -height/2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}

Vector Entity::getVector(float vX, float vY){
	Vector vec;
	float newX = vX;
	float newY = vY;
	newX *= scaleX;
	vX *= scaleX;
	newY *= scaleY;
	vY *= scaleY;
	newX = -vY*sin(rotation*PI / 180) + vX*cos(rotation*PI / 180);
	newY = vY*cos(rotation*PI / 180) + vX*sin(rotation*PI / 180);
	newX += x;
	newY += y;

	vec.x = newX;
	vec.y = newY;
	return vec;
}

std::vector<Vector> Entity::getVertices(){
	std::vector<Vector> vv;

	Vector v1 = getVector(-width / 2, height / 2);
	vv.push_back(v1);

	Vector v2 = getVector(width / 2, height / 2);
	vv.push_back(v2);

	Vector v3 = getVector(width / 2, -height / 2);
	vv.push_back(v3);

	Vector v4 = getVector(-width / 2, -height / 2);
	vv.push_back(v4);

	return vv;
}

