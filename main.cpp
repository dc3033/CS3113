#pragma once
#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include "Entity.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#define MAX_TIMESTEPS 6
#define FIXED_TIMESTEP 0.016666f

using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint fontSheet;
GLuint sprite;
Entity player;
Entity rRectangle;
Entity pRectangle;
Entity bSquare;
Entity gSquare;

bool rLeft = true;
bool pUp = true;
bool bGrow = true;
bool gLeft = true;
bool gGrow = true;

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SDL_FreeSurface(surface);
	return textureID;
}

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++){
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing)*i) + (-0.5f * size), 0.5f * size,
			((size + spacing)*i) + (-0.5f * size), -0.5f * size,
			((size + spacing)*i) + (0.5f * size), 0.5f * size,
			((size + spacing)*i) + (0.5f * size), -0.5f * size,
			((size + spacing)*i) + (0.5f * size), 0.5f * size,
			((size + spacing)*i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size
		});

	}
	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

}

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];
	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p < 0) {
		return true;
	}
	return false;
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) {
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}

		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	return true;
}

float lastFrameTicks = 0.0f;
float timeLeftOver = 0.0f;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("SAT Collision Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 800, 600);

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-1.33f, 1.33f, -1.0f, 1.0f, -1.0f, 1.0f);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	fontSheet = LoadTexture("font1.png");

	sprite = LoadTexture("element_yellow_square_glossy.png");
	player.sprite = SheetSprite(sprite);
	player.width = 0.1;
	player.height = 0.1;
	player.x = 0.0;
	player.y = 0.0;
	player.rotation = 45;

	sprite = LoadTexture("element_red_square.png");
	rRectangle.sprite = SheetSprite(sprite);
	rRectangle.width = 0.1;
	rRectangle.height = 0.3;
	rRectangle.x = 0.5;
	rRectangle.y = 0.5;
	rRectangle.rotation = 60;

	sprite = LoadTexture("element_purple_rectangle_glossy.png");
	pRectangle.sprite = SheetSprite(sprite);
	pRectangle.width = 0.3;
	pRectangle.height = 0.15;
	pRectangle.x = -0.5;
	pRectangle.y = -0.5;
	pRectangle.rotation = 30;

	sprite = LoadTexture("element_blue_square_glossy.png");
	bSquare.sprite = SheetSprite(sprite);
	bSquare.width = 0.2;
	bSquare.height = 0.2;
	bSquare.x = -0.5;
	bSquare.y = 0.5;
	bSquare.rotation = 75;

	sprite = LoadTexture("element_green_square_glossy.png");
	gSquare.sprite = SheetSprite(sprite);
	gSquare.width = 0.2;
	gSquare.height = 0.2;
	gSquare.x = 0.5;
	gSquare.y = -0.5;
	gSquare.rotation = 105;


	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		float fixedElapsed = elapsed + timeLeftOver;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}

		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
		
			const Uint8 *keys = SDL_GetKeyboardState(NULL);

			if (keys[SDL_SCANCODE_W]){
				player.velocityY = 1.0f;
			}
			else if (keys[SDL_SCANCODE_S]){
				player.velocityY = -1.0f;
			}
			else { player.velocityY = 0.0f; }

			if (keys[SDL_SCANCODE_D]){
				player.velocityX = 1.0f;
			}
			else if (keys[SDL_SCANCODE_A]){
				player.velocityX = -1.0f;
			}
			else { player.velocityX = 0.0f; }

			if (keys[SDL_SCANCODE_Q]){
				player.rotation += 5.0f;
			}
			if (keys[SDL_SCANCODE_E]){
				player.rotation -= 5.0f;
			}

			player.x += player.velocityX * FIXED_TIMESTEP;
			player.y += player.velocityY * FIXED_TIMESTEP;

			if (rRectangle.x < 0.2) rLeft = false;
			if (rRectangle.x > 1.0) rLeft = true;
			if (rLeft) rRectangle.velocityX = -0.5;
			else rRectangle.velocityX = 0.5;

			rRectangle.x += rRectangle.velocityX * FIXED_TIMESTEP;

			if (pRectangle.y > 0) pUp = false;
			if (pRectangle.y < -0.8) pUp = true;
			if (pUp) pRectangle.velocityY = 0.5;
			else pRectangle.velocityY = -0.5;

			pRectangle.y += pRectangle.velocityY * FIXED_TIMESTEP;

			if (bSquare.scaleX > 1.5) bGrow = false;
			if (bSquare.scaleX < 0.5) bGrow = true;
			if (bGrow) bSquare.scaleX = bSquare.scaleY += 0.01;
			else bSquare.scaleX = bSquare.scaleY -= 0.01;

			if (gSquare.x < 0.2) gLeft = false;
			if (gSquare.x > 1.0) gLeft = true;
			if (gLeft) {
				gSquare.velocityX = -0.5;
				gSquare.velocityY = 0.5;
			}
			else {
				gSquare.velocityX = 0.5;
				gSquare.velocityY = -0.5;
			}
			if (gSquare.scaleX > 1.5) gGrow = false;
			if (gSquare.scaleX < 0.5) gGrow = true;
			if (gGrow) gSquare.scaleX = gSquare.scaleY += 0.01;
			else gSquare.scaleX = gSquare.scaleY -= 0.01;

			gSquare.x += gSquare.velocityX * FIXED_TIMESTEP;
			gSquare.y += gSquare.velocityY * FIXED_TIMESTEP;

		}

		timeLeftOver = fixedElapsed;

		if (checkSATCollision(player.getVertices(), rRectangle.getVertices())){
			if ((player.velocityX + rRectangle.velocityX) > 1){
				player.x -= player.velocityX * FIXED_TIMESTEP;
				player.y -= player.velocityY * FIXED_TIMESTEP;
				player.velocityX = rRectangle.velocityX;
				player.velocityY = 0;
				player.x += player.velocityX * FIXED_TIMESTEP;
				player.y += player.velocityY * FIXED_TIMESTEP;
			}
			else if (player.velocityX == 0){
				player.x -= player.velocityX * FIXED_TIMESTEP;
				player.y -= player.velocityY * FIXED_TIMESTEP;
				player.velocityX = rRectangle.velocityX;
				player.velocityY = 0;
				player.x += player.velocityX * FIXED_TIMESTEP;
				player.y += player.velocityY * FIXED_TIMESTEP;
			}
			else {
				player.x -= player.velocityX * FIXED_TIMESTEP;
				player.y -= player.velocityY * FIXED_TIMESTEP;
				player.velocityX = 0;
				player.velocityY = 0;
				rRectangle.x -= rRectangle.velocityX * FIXED_TIMESTEP;
				rRectangle.velocityX = 0;
			}
		}

		if (checkSATCollision(player.getVertices(), pRectangle.getVertices())){
			if ((player.velocityY + pRectangle.velocityY) > 1){
				player.x -= player.velocityX * FIXED_TIMESTEP;
				player.y -= player.velocityY * FIXED_TIMESTEP;
				player.velocityX = 0;
				player.velocityY = pRectangle.velocityY;
				player.x += player.velocityX * FIXED_TIMESTEP;
				player.y += player.velocityY * FIXED_TIMESTEP;
			}
			else if (player.velocityY == 0){
				player.x -= player.velocityX * FIXED_TIMESTEP;
				player.y -= player.velocityY * FIXED_TIMESTEP;
				player.velocityX = 0;
				player.velocityY = pRectangle.velocityY;
				player.x += player.velocityX * FIXED_TIMESTEP;
				player.y += player.velocityY * FIXED_TIMESTEP;
			}
			else {
				player.x -= player.velocityX * FIXED_TIMESTEP;
				player.y -= player.velocityY * FIXED_TIMESTEP;
				player.velocityX = 0;
				player.velocityY = 0;
				pRectangle.y -= pRectangle.velocityY * FIXED_TIMESTEP;
				pRectangle.velocityY = 0;
			}
		}

		if (checkSATCollision(player.getVertices(), bSquare.getVertices())){
			player.x -= player.velocityX * FIXED_TIMESTEP;
			player.y -= player.velocityY * FIXED_TIMESTEP;
			player.velocityX = 0;
			player.velocityY = 0;
			if (bGrow) bSquare.scaleY -= 0.01;
			else bSquare.scaleY += 0.01;
		}

		if (checkSATCollision(player.getVertices(), gSquare.getVertices())){
			player.x -= player.velocityX * FIXED_TIMESTEP;
			player.y -= player.velocityY * FIXED_TIMESTEP;
			player.velocityX = 0;
			player.velocityY = 0;
			gSquare.x -= gSquare.velocityX * FIXED_TIMESTEP;
			gSquare.y -= gSquare.velocityY * FIXED_TIMESTEP;
			gSquare.velocityX = 0;
			gSquare.velocityY = 0;
			if (gGrow) gSquare.scaleY -= 0.01;
			else gSquare.scaleY += 0.01;
		}

		glClearColor(0.1f, 0.2f, 0.7f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

        modelMatrix.identity();
		modelMatrix.Translate(player.x, player.y, 0);
		modelMatrix.Scale(player.scaleX, player.scaleY, 1.0);
		modelMatrix.Rotate(player.rotation*PI / 180);
		program.setModelMatrix(modelMatrix);
		player.draw(program);
		
		modelMatrix.identity();
		modelMatrix.Translate(rRectangle.x, rRectangle.y, 0);
		modelMatrix.Scale(rRectangle.scaleX, rRectangle.scaleY, 1.0);
		modelMatrix.Rotate(rRectangle.rotation*PI / 180);
		program.setModelMatrix(modelMatrix);
		rRectangle.draw(program);

		modelMatrix.identity();
		modelMatrix.Translate(pRectangle.x, pRectangle.y, 0);
		modelMatrix.Scale(pRectangle.scaleX, pRectangle.scaleY, 1.0);
		modelMatrix.Rotate(pRectangle.rotation*PI / 180);
		program.setModelMatrix(modelMatrix);		
		pRectangle.draw(program);

		modelMatrix.identity();
		modelMatrix.Translate(bSquare.x, bSquare.y, 0);
		modelMatrix.Scale(bSquare.scaleX, bSquare.scaleY, 1.0);
		modelMatrix.Rotate(bSquare.rotation*PI / 180);
		program.setModelMatrix(modelMatrix);
		
		bSquare.draw(program);

		modelMatrix.identity();
		modelMatrix.Translate(gSquare.x, gSquare.y, 0);
		modelMatrix.Scale(gSquare.scaleX, gSquare.scaleY, 1.0);
		modelMatrix.Rotate(gSquare.rotation*PI / 180);
		program.setModelMatrix(modelMatrix);

		gSquare.draw(program);

		modelMatrix.identity();
		modelMatrix.Translate(-1.3, 0.9, 0);
		program.setModelMatrix(modelMatrix);
		DrawText(&program, fontSheet, "Sorry it's only rectangles, but the collision is mostly on point", 0.07, -0.03);

		modelMatrix.identity();
		modelMatrix.Translate(-1.2, 0.8, 0);
		program.setModelMatrix(modelMatrix);
		DrawText(&program, fontSheet, "WASD to move, Q and E to rotate", 0.1, -0.02);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
