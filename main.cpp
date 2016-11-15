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
#define LEVEL_HEIGHT 32
#define LEVEL_WIDTH 128
#define TILE_SIZE 0.05f
#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

int mapWidth;
int mapHeight;
unsigned char** levelData;
GLuint fontSheet;
GLuint spriteSheet;
Entity player;
vector<Entity> coinVector;
int score = 0;

enum GameState {STATE_GAME_LEVEL, STATE_WIN_GAME, STATE_LOSE_GAME};

int state = STATE_GAME_LEVEL;

float gravity = -0.2;

bool readHeader(std::ifstream &stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }

		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);

		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}

	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else {
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool readLayerData(std::ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < LEVEL_HEIGHT; y++) {
				getline(stream, line);
				istringstream lineStream(line); 
				string tile;

				for (int x = 0; x < LEVEL_WIDTH; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

void placeEntity(string type, float placeX, float placeY){
	if (type == "Player"){
		player.x = placeX;
		player.y = placeY;
	}
	if (type == "Coin"){
		Entity coin;
		coin.sprite = SheetSprite(spriteSheet, 78, 30, 30);
		coin.x = placeX;
		coin.y = placeY;
		coinVector.push_back(coin);
	}
}

bool readEntityData(std::ifstream &stream) {

	string line;
	string type;

	while (getline(stream, line)) {
		if (line == "") { break; }

		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);

		if (key == "type"){
			type = value;
		}
		else if (key == "location") {

			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');

			float placeX = atoi(xPosition.c_str()) / 16 * TILE_SIZE;
			float placeY = atoi(yPosition.c_str()) / 16 * -TILE_SIZE;

			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}

void buildLevel(){
	ifstream infile("hwMap.txt");
	string line;
	while (getline(infile, line)) {

		if (line == "[header]") {
			if (!readHeader(infile)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[ObjectLayer]") {
			readEntityData(infile);
		}
	}
}

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

bool isSolid(unsigned int tile){
	switch (tile){
	case 121:
	case 122:
	case 123:
	case 152:
	case 154:
	case 301:
	case 302:
	case 303:
	case 332:
		return true;
		break;
	default:
		return false;
		break;
	}
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY)
{
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}

float checkCollisionY(float x, float y)
{
	int gridX, gridY;
	worldToTileCoordinates(x, y, &gridX, &gridY);
	if (gridX < 0 || gridX > 128 || gridY < 0 || gridY > 40)
		return 0.0f;

	if (isSolid(levelData[gridY][gridX]))
	{
		float yCoord = (gridY * TILE_SIZE) - (TILE_SIZE*0.0f);
		return -y - yCoord;
	}
	return 0.0;
}

float checkCollisionX(float x, float y)
{
	int gridX, gridY;
	worldToTileCoordinates(x, y, &gridX, &gridY);
	if (gridX < 0 || gridX > 128 || gridY < 0 || gridY > 40)
		return 0.0f;

	if (isSolid(levelData[gridY][gridX]))
	{
		float xCoord = (gridX * TILE_SIZE) - (TILE_SIZE*0.0f);
		return x - xCoord;
	}
	return 0.0;
}

void levelCollisionY(Entity* entity)
{
	//check bottom
	float adjust = checkCollisionY(entity->x, entity->y - entity->sprite.height * 0.5f + 0.03);
	if (adjust != 0.0f)
	{
		entity->y += adjust;
		entity->velocityY = 0.0f;
		entity->collideBottom = true;
	}

	//check top
	adjust = checkCollisionY(entity->x, entity->y + entity->sprite.height * 0.5f - 0.03);
	if (adjust != 0.0f)
	{
		entity->y += adjust - TILE_SIZE;
		entity->velocityY = 0.0f;
		entity->collideTop = true;
	}
}

void levelCollisionX(Entity* entity)
{
	//check left
	float adjust = checkCollisionX(entity->x - entity->sprite.width * 0.5f, entity->y);
	if (adjust != 0.0f)
	{
		entity->x += adjust * TILE_SIZE;
		entity->velocityX = 0.0f;
		entity->collideLeft = true;
	}

	//check right
	adjust = checkCollisionX(entity->x + entity->sprite.width * 0.5f, entity->y);
	if (adjust != 0.0f)
	{
		entity->x += (adjust - TILE_SIZE) * TILE_SIZE;
		entity->velocityX = 0.0f;
		entity->collideRight = true;
	}
}

void collisionDetect(){
	if (state != STATE_GAME_LEVEL)
		return;

	if (player.y < -1.5){
		player.alive = false;
		state = STATE_LOSE_GAME;
	}
	if (player.x > 6.33){
		state = STATE_WIN_GAME;
	}
	if (player.x < 0.0){
		player.x += .01;
		player.velocityX = 0.0f;
	}
}

float lastFrameTicks = 0.0f;
float timeLeftOver = 0.0f;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Coin Getter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
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
	spriteSheet = LoadTexture("spritesheet_rgba.png");
	player.sprite = SheetSprite(spriteSheet, 19, 30, 30);
	
	buildLevel();

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
			if (state == STATE_GAME_LEVEL){

				player.collideBottom = false;
				player.collideTop = false;
				player.collideLeft = false;
				player.collideRight = false;

				player.velocityX += player.accelerationX * FIXED_TIMESTEP;
				player.velocityY += player.accelerationY * FIXED_TIMESTEP;
				player.velocityY += gravity * FIXED_TIMESTEP;

				levelCollisionY(&player);
				levelCollisionX(&player);

				const Uint8 *keys = SDL_GetKeyboardState(NULL);

				if (keys[SDL_SCANCODE_D]){
					player.velocityX = 1.0f;
				}
				else if (keys[SDL_SCANCODE_A]){
					player.velocityX = -1.0f;
				}
				else { player.velocityX = 0.0f; }
				if (player.collideBottom){
					if (keys[SDL_SCANCODE_SPACE]){
						player.velocityY = 1.0f;
					}
				}
				player.x += player.velocityX * FIXED_TIMESTEP;
				player.y += player.velocityY * FIXED_TIMESTEP;
			}
		}

		timeLeftOver = fixedElapsed;

		collisionDetect();

		for (int i = 0; i < coinVector.size(); ++i){
			if (coinVector[i].collideEntity(&player)){
				coinVector[i].alive = false;
				score += 1;
			}
		}

		glClearColor(0.1f, 0.2f, 0.7f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		if (state == STATE_GAME_LEVEL){
			float translateX = player.x;
			float translateY = player.y/2;
			if (translateX > 0.0) translateX = 0.0;
			if (translateY < 0.0) translateY = 0.0;

			viewMatrix.identity();
			viewMatrix.Translate(translateX, translateY, 0.0f);
			glBindTexture(GL_TEXTURE_2D, spriteSheet);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			vector<float> vertexData;
			vector<float> texCoordData;
			for (int y = 0; y < mapHeight; y++) {
				for (int x = 0; x < mapWidth; x++) {
					if (levelData[y][x] != 0){
						float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
						float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
						float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
						float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
						vertexData.insert(vertexData.end(), {
							TILE_SIZE * x, -TILE_SIZE * y,
							TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
							(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y,
							(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
							(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y,
							TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE
						});
						texCoordData.insert(texCoordData.end(), { 
							u, v,
							u, v + spriteHeight,
							u + spriteWidth, v,
							u + spriteWidth, v + spriteHeight,
							u + spriteWidth, v,
							u, v + spriteHeight
						});
					}
				}
				
			}
			//viewMatrix.Translate(TILE_SIZE * LEVEL_WIDTH / 2, -TILE_SIZE * LEVEL_HEIGHT / 2, 0.0f);
			glUseProgram(program.programID);

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
			glEnableVertexAttribArray(program.positionAttribute);

			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
			glEnableVertexAttribArray(program.texCoordAttribute);

			//viewMatrix.Translate(-TILE_SIZE*mapWidth / 2, TILE_SIZE*mapHeight / 2, 0.0f);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);
			
			for (int i = 0; i < coinVector.size(); ++i){
				if (coinVector[i].alive){
					modelMatrix.identity();
					modelMatrix.Translate(coinVector[i].x, coinVector[i].y, 0.0);
					coinVector[i].draw(program);
				}
			}
			modelMatrix.identity();
			modelMatrix.Translate(player.x, player.y, 0.0);
			player.draw(program);
		}
		else if (state == STATE_WIN_GAME){
			viewMatrix.identity();
			viewMatrix.Translate(-0.2f, 0.2f, 0.0f);
			DrawText(&program, fontSheet, "YOU WIN!", 0.05f, 0.0f);
		}
		else if (state == STATE_LOSE_GAME){
			viewMatrix.identity();
			viewMatrix.Translate(-0.2f, 0.2f, 0.0f);
			DrawText(&program, fontSheet, "YOU DIED", 0.05f, 0.0f);
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
