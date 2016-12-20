#pragma once
#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
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
#define LEVEL_HEIGHT 24
#define LEVEL_WIDTH 32
#define TILE_SIZE 0.083f
#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

Mix_Chunk *chooseSound;
Mix_Chunk *flipSound;
Mix_Chunk *hurtSound;
Mix_Chunk *jumpSound;
Mix_Chunk *shotSound;
Mix_Chunk *laserSound;
Mix_Chunk *boomSound;
Mix_Chunk *explosionSound;
Mix_Chunk *bigsplosionSound;
Mix_Music *menuMusic;
Mix_Music *gameMusic;

int mapWidth;
int mapHeight;
short int** levelData;

GLuint fontSheet;
GLuint platformerSheet;
GLuint sprite;
Entity indicator;
Entity decoFlag1;
Entity decoFlag2;
Entity player1;
Entity player2;
vector<Entity> flagVector;
vector<Entity> bulletVector;
vector<Entity> explosionVector;

float p1score;
int p1kills;
int p1health;
float p1energy;
float p1RespawnX;
float p1RespawnY;
float p1Cooldown;
float p1GodMode;
bool p1win;
int p1walkState;
float p1walkCycler;

float p2score;
int p2kills;
int p2health;
float p2energy;
float p2RespawnX;
float p2RespawnY;
float p2Cooldown;
float p2GodMode;
bool p2win;
int p2walkState;
float p2walkCycler;

float gravity = -1.0;

enum WalkStage {WS_ONE, WS_TWO, WS_THREE, WS_FOUR};

enum GameState {STATE_START, STATE_MAIN_MENU, STATE_CONTROLS, STATE_RULES, STATE_MAP_SELECT, STATE_GAME_LEVEL, STATE_GAME_OVER};

enum MenuChoice {LEVEL_SELECT, RULES_CONTROLS};

enum MapChoice {MAP_ONE, MAP_TWO, MAP_THREE};

int gameState = STATE_START;
int menuChoice = LEVEL_SELECT;
int mapChoice = MAP_ONE;

void gameStatReset(){
	p1score = 0;
	p1kills = 0;
	p1health = 10;
	p1energy = 0.00;
	p1RespawnX = player1.x;
	p1RespawnY = player1.y;
	p1GodMode = 3.0;
	p1win = false;

	p2score = 0.00;
	p2kills = 0;
	p2health = 10;
	p2energy = 0.00;
	p2RespawnX = player2.x;
	p2RespawnY = player2.y;
	p2GodMode = 3.0;
	p2win = false;
}

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
		levelData = new short int*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new short int[mapWidth];
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
					short int val = (short int)atoi(tile.c_str());
					if (val != 0) {
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
	if (placeX < TILE_SIZE * 16) { placeX = (-TILE_SIZE * 16) + placeX; }
	else { placeX = placeX - (TILE_SIZE * 16); }

	if (placeY < TILE_SIZE * 12) { placeY = (TILE_SIZE * 12) - placeY; }
	else { placeY = -placeY + (TILE_SIZE * 12); }

	if (type == "Player1"){
		player1.x = placeX + TILE_SIZE/2;
		player1.y = placeY + TILE_SIZE/2 + 0.02;
	}
	if (type == "Player2"){
		player2.x = placeX + TILE_SIZE / 2;
		player2.y = placeY + TILE_SIZE / 2 + 0.02;
	}
	if (type == "Flag"){
		Entity flag;
		flag.sprite = SheetSprite(platformerSheet, 310, 30, 30);
		flag.x = placeX + TILE_SIZE / 2;
		flag.y = placeY + TILE_SIZE / 2;
		flagVector.push_back(flag);
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

			float placeX = atoi(xPosition.c_str()) * TILE_SIZE;
			float placeY = atoi(yPosition.c_str()) * TILE_SIZE;

			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}

void buildLevel(string levelName){
	ifstream infile(levelName);
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
		else if (line == "[Object Layer 1]") {
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
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY)
{
	*gridX = (int)(16 + (worldX / TILE_SIZE));
	*gridY = (int)(12 + (-worldY / TILE_SIZE));
}

float checkCollisionTop(float x, float y)
{
	int gridX, gridY;
	worldToTileCoordinates(x, y, &gridX, &gridY);

	if ((levelData[gridY][gridX]) > 0)
	{
		float yCoord = -((gridY-11)*TILE_SIZE);
		return yCoord - y;
	}
	return 0.0;
}

float checkCollisionBot(float x, float y)
{
	int gridX, gridY;
	worldToTileCoordinates(x, y, &gridX, &gridY);

	if ((levelData[gridY][gridX]) > 0)
	{
		float yCoord = -((gridY - 12)*TILE_SIZE);
		return yCoord - y;
	}
	return 0.0;
}

float checkCollisionLeft(float x, float y)
{
	int gridX, gridY;
	worldToTileCoordinates(x, y, &gridX, &gridY);

	if ((levelData[gridY][gridX]) > 0)
	{
		float xCoord = (gridX-15)*TILE_SIZE;
		return xCoord - x;
	}
	return 0.0;
}

float checkCollisionRight(float x, float y)
{
	int gridX, gridY;
	worldToTileCoordinates(x, y, &gridX, &gridY);

	if ((levelData[gridY][gridX]) > 0)
	{
		float xCoord = (gridX-16)*TILE_SIZE;
		return xCoord - x;
	}
	return 0.0;
}

void levelCollisionY(Entity* entity)
{
	//check bottom
	float adjust = checkCollisionBot(entity->x, entity->y - 0.04);
	if (adjust != 0.0f)
	{
		entity->y += adjust;
		entity->velocityY = 0.0f;
		entity->accelerationY = 0.0f;
		entity->collideBottom = true;
	}

	//check top
	adjust = checkCollisionTop(entity->x, entity->y + 0.04);
	if (adjust != 0.0f)
	{
		entity->y += adjust;
		entity->velocityY = 0.0f;
		entity->collideTop = true;
	}
}

void levelCollisionX(Entity* entity)
{
	//check left
	float adjust = checkCollisionLeft(entity->x - 0.04, entity->y);
	if (adjust != 0.0f)
	{
		entity->x += adjust;
		entity->velocityX = 0.0f;
		entity->collideLeft = true;
	}

	//check right
	adjust = checkCollisionRight(entity->x + 0.04, entity->y);
	if (adjust != 0.0f)
	{
		entity->x += adjust;
		entity->velocityX = 0.0f;
		entity->collideRight = true;
	}
}

void bulletLevelCollisionX(Entity* bullet){
	//check left
	float adjust = checkCollisionLeft(bullet->x - 0.01, bullet->y);
	if (adjust != 0.0f)
	{
		bullet->alive = 0;
		bullet->velocityX = 0.0f;
		bullet->collideLeft = true;
	}

	//check right
	adjust = checkCollisionRight(bullet->x + 0.01, bullet->y);
	if (adjust != 0.0f)
	{
		bullet->alive = 0;
		bullet->velocityX = 0.0f;
		bullet->collideRight = true;
	}
}

void playerUpdate(Entity& player){
	if (gameState == STATE_GAME_LEVEL){
		player.collideBottom = false;
		player.collideTop = false;
		player.collideLeft = false;
		player.collideRight = false;

		player.velocityX += player.accelerationX * FIXED_TIMESTEP;
		player.velocityY += player.accelerationY * FIXED_TIMESTEP;
		player.accelerationY += gravity * FIXED_TIMESTEP;

		levelCollisionY(&player);
		levelCollisionX(&player);
	}
}

void flagUpdate(Entity& flag){
	if (gameState == STATE_GAME_LEVEL){
		if (flag.collideEntity(&player1)){
			if (flag.p1CaptureStatus >= 3 && flag.status != 2){
				flag.p1CaptureStatus = 3;
				flag.status = 2;
				flag.sprite = SheetSprite(platformerSheet, 312, 30, 30);
				Mix_PlayChannel(-1, flipSound, 0);
			}
			else if (flag.p1CaptureStatus >= 3 && flag.status == 2){
				flag.p1CaptureStatus = 3;
			}
			else if (flag.status == 1){
				flag.p2CaptureStatus -= 0.03;
			}
			else if (flag.status == 0 || flag.status == 2){
				flag.p1CaptureStatus += 0.03;
			}
		}
		if (flag.collideEntity(&player2)){
			if (flag.p2CaptureStatus >= 3 && flag.status != 1){
				flag.p2CaptureStatus = 3;
				flag.status = 1;
				flag.sprite = SheetSprite(platformerSheet, 313, 30, 30);
				Mix_PlayChannel(-1, flipSound, 0);
			}
			else if (flag.p2CaptureStatus >= 3 && flag.status == 1){
				flag.p2CaptureStatus = 3;
			}
			else if (flag.status == 2){
				flag.p1CaptureStatus -= 0.03;
			}
			else if (flag.status == 0 || flag.status == 1){
				flag.p2CaptureStatus += 0.03;
			}
		}
		if (flag.p1CaptureStatus < 0.1 && flag.p2CaptureStatus < 0.1){
			flag.status = 0;
			flag.sprite = SheetSprite(platformerSheet, 310, 30, 30);
		}
		if (flag.status == 1){ p2score += 0.025; }
		else if (flag.status == 2){ p1score += 0.025; }
	}
}

void shootShot(Entity& player){
	Entity shot;
	shot.bType = 1;
	shot.sprite = SheetSprite(platformerSheet, 711, 30, 30);
	shot.resize(0.015);
	if (player.direction == 1){
		shot.x = player.x + 0.065;
		shot.velocityX = 1.5;
	}
	else {
		shot.x = player.x - 0.065;
		shot.velocityX = -1.5;
	}
	shot.y = player.y;
	bulletVector.push_back(shot);
}

void shootLaser(Entity& player){
	Entity laser;
	laser.bType = 2;
	if (player.status == 2){
		laser.sprite = SheetSprite(platformerSheet, 894, 30, 30);
	}
	else { laser.sprite = SheetSprite(platformerSheet, 895, 30, 30); }
	laser.resize(0.015);
	if (player.direction == 1){
		laser.x = player.x + 0.065;
		laser.velocityX = 7.5;
	}
	else {
		laser.x = player.x - 0.065;
		laser.velocityX = -7.5;
	}
	laser.y = player.y;
	bulletVector.push_back(laser);
}

void shootBoom(Entity& player){
	Entity Boom;
	Boom.bType = 3;
	if (player.status == 2){
		Boom.sprite = SheetSprite(platformerSheet, 896, 30, 30);
	}
	else { Boom.sprite = SheetSprite(platformerSheet, 897, 30, 30); }
	Boom.resize(0.015);
	if (player.direction == 1){
		Boom.x = player.x + 0.065;
		Boom.velocityX = 3;
	}
	else {
		Boom.x = player.x - 0.065;
		Boom.velocityX = -3;
	}
	Boom.y = player.y;
	bulletVector.push_back(Boom);
}

void boomExplosion(Entity& boom){
	Entity explosion;
	explosion.boomType = 0;
	explosion.sprite = SheetSprite(platformerSheet, 626, 30, 30);
	explosion.x = boom.x;
	explosion.y = boom.y;
	explosion.lifeTime = 0.2;
	explosion.resize(explosion.lifeTime);
	explosionVector.push_back(explosion);
}

void selfDestruct(Entity& player){
	Entity explosion;
	explosion.boomType = 1;
	explosion.sprite = SheetSprite(platformerSheet, 626, 30, 30);
	explosion.x = player.x;
	explosion.y = player.y;
	explosion.lifeTime = 1.0;
	explosion.resize(explosion.lifeTime);
	explosionVector.push_back(explosion);
}

void bulletUpdate(Entity& bullet){
	if (gameState == STATE_GAME_LEVEL){
		if (bullet.collideEntity(&player1)){
			if (p1GodMode <= 0){
				if (bullet.bType == 1) {
					p1health -= 1;
				}
				else if (bullet.bType == 2) {
					p1health -= 2;
				}
				else if (bullet.bType == 3) {
					p1health -= 3;
				}
				Mix_PlayChannel(-1, hurtSound, 0);
			}
			bullet.alive = 0;
		}
		else if (bullet.collideEntity(&player2)){
			if (p2GodMode <= 0){
				if (bullet.bType == 1) {
					p2health -= 1;
				}
				else if (bullet.bType == 2) {
					p2health -= 2;
				}
				else if (bullet.bType == 3) {
					p2health -= 3;
				}
				Mix_PlayChannel(-1, hurtSound, 0);
			}
			bullet.alive = 0;
		}
		bulletLevelCollisionX(&bullet);
		bullet.x += bullet.velocityX * FIXED_TIMESTEP;
	}
}

bool shouldRemoveBullet(Entity& bullet) {
	if (bullet.alive == 0) {
		return true;
	}
	else return false;
}

void explosionUpdate(Entity& explosion){
	if (gameState == STATE_GAME_LEVEL){
		if (explosion.boomType == 0){
			if (explosion.collideEntity(&player1)){
				if (explosion.lifeTime > 0.19 && p1GodMode <= 0) {
					p1health -= 2;
				}
				Mix_PlayChannel(-1, hurtSound, 0);
			}
			else if (explosion.collideEntity(&player2)){
				if (explosion.lifeTime > 0.19 && p2GodMode <= 0) {
					p2health -= 2;
				}
				Mix_PlayChannel(-1, hurtSound, 0);
			}
			explosion.lifeTime -= 0.01;
			explosion.resize(explosion.lifeTime);
			if (explosion.lifeTime <= 0.02){
				explosion.alive = 0;
				Mix_PlayChannel(-1, explosionSound, 0);
			}
		}
		else {
			if (explosion.collideEntity(&player1)){
				if (explosion.lifeTime > 0.99 && p1GodMode <= 0) {
					p1health -= 8;
				}
				Mix_PlayChannel(-1, hurtSound, 0);
			}
			else if (explosion.collideEntity(&player2)){
				if (explosion.lifeTime > 0.99 && p2GodMode <= 0) {
					p2health -= 8;
				}
				Mix_PlayChannel(-1, hurtSound, 0);
			}
			explosion.lifeTime -= 0.01;
			explosion.resize(explosion.lifeTime);
			if (explosion.lifeTime <= 0.1){
				explosion.alive = 0;
			}
		}
	}
}

bool shouldRemoveExplosion(Entity& explosion) {
	if (explosion.alive == 0) {
		return true;
	}
	else return false;
}

float lastFrameTicks = 0.0f;
float timeLeftOver = 0.0f;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Flag Cappers", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
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
	platformerSheet = LoadTexture("spritesheet_rgba.png");
	indicator.sprite = SheetSprite(platformerSheet, 169, 30, 30);
	player1.sprite = SheetSprite(platformerSheet, 88, 30, 30);
	player1.status = 2;
	player2.sprite = SheetSprite(platformerSheet, 58, 30, 30);
	player2.status = 1;
	player2.direction = -1;

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	chooseSound = Mix_LoadWAV("choose.wav");
	flipSound = Mix_LoadWAV("select.wav");
	jumpSound = Mix_LoadWAV("jump.wav");
	hurtSound = Mix_LoadWAV("hurt.wav");
	shotSound = Mix_LoadWAV("shot.wav");
	laserSound = Mix_LoadWAV("laser.wav");
	boomSound = Mix_LoadWAV("boomshot.wav");
	explosionSound = Mix_LoadWAV("explosion.wav");
	bigsplosionSound = Mix_LoadWAV("bigsplosion.wav");
	menuMusic = Mix_LoadMUS("Main Theme From Metal Slug.mp3");
	gameMusic = Mix_LoadMUS("Stage 1 Contra Music.mp3");

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
		
			if (gameState == STATE_GAME_LEVEL){
				playerUpdate(player1);
				playerUpdate(player2);

				for (int i = 0; i < flagVector.size(); ++i){
					flagUpdate(flagVector[i]);
				}

				for (int i = 0; i < bulletVector.size(); ++i){
					bulletUpdate(bulletVector[i]);
					if (bulletVector[i].alive == 0 && bulletVector[i].bType == 3){
						boomExplosion(bulletVector[i]);
					}
				}

				for (int i = 0; i < explosionVector.size(); ++i){
					explosionUpdate(explosionVector[i]);
				}

				bulletVector.erase((remove_if(bulletVector.begin(), bulletVector.end(), shouldRemoveBullet)), bulletVector.end());
				explosionVector.erase((remove_if(explosionVector.begin(), explosionVector.end(), shouldRemoveExplosion)), explosionVector.end());

				if (p1health <= 0) {
					p2score += 10;
					p2kills += 1;
					p1health = 10;
					player1.x = p1RespawnX;
					player1.y = p1RespawnY;
					p1energy = 0;
					p1Cooldown = 2.0;
					p1GodMode = 3.0;
				}
				else {
					if (p1GodMode > 0) p1GodMode -= 0.03;
					if (p1Cooldown > 0) p1Cooldown -= 0.04;
				}

				if (p2health <= 0) {
					p1score += 10;
					p1kills += 1;
					p2health = 10;
					player2.x = p2RespawnX;
					player2.y = p2RespawnY;
					p2energy = 0;
					p2Cooldown = 2.0;
					p2GodMode = 3.0;
				}
				else {
					if (p2GodMode > 0) p2GodMode -= 0.03;
					if (p2Cooldown > 0) p2Cooldown -= 0.04;
				}

				if (p1score >= 50) {
					p1win = true;
					gameState = STATE_GAME_OVER;
				}
				else if (p2score >= 50) {
					p2win = true;
					gameState = STATE_GAME_OVER;
				}

				if (p1energy <= 10) p1energy += 0.05;
				if (p2energy <= 10) p2energy += 0.05;

			}



			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			//Start screen to initialize music
			if (gameState == STATE_START){
				
				if (keys[SDL_SCANCODE_SPACE]){
					Mix_PlayMusic(menuMusic, -1);
					gameState = STATE_MAIN_MENU;
				}
			}

			//Main menu choice select
			if (gameState == STATE_MAIN_MENU) {

				if (menuChoice == 1) {
					if (keys[SDL_SCANCODE_W]){
						menuChoice = 0;
					}
					if (keys[SDL_SCANCODE_RETURN]){
						gameState = STATE_CONTROLS;
						Mix_PlayChannel(-1, chooseSound, 0);
					}
				}
				if (menuChoice == 0) {
					if (keys[SDL_SCANCODE_S]){
						menuChoice = 1;
					}
					if (keys[SDL_SCANCODE_RETURN]){
						gameState = STATE_MAP_SELECT;
						Mix_PlayChannel(-1, chooseSound, 0);
					}
				}
				if (keys[SDL_SCANCODE_KP_MINUS]){
					exit(-1);
				}
			}

			//Controls screen controls
			if (gameState == STATE_CONTROLS){
				if (keys[SDL_SCANCODE_ESCAPE]){
					gameState = STATE_MAIN_MENU;
					Mix_PlayChannel(-1, chooseSound, 0);
				}
				if (keys[SDL_SCANCODE_SPACE]){
					gameState = STATE_RULES;
					Mix_PlayChannel(-1, chooseSound, 0);
				}
				if (keys[SDL_SCANCODE_KP_MINUS]){
					exit(-1);
				}
			}

			//Rules screen controls
			if (gameState == STATE_RULES){
				if (keys[SDL_SCANCODE_ESCAPE]){
					gameState = STATE_MAIN_MENU;
					Mix_PlayChannel(-1, chooseSound, 0);
				}
				if (keys[SDL_SCANCODE_RETURN]){
					gameState = STATE_CONTROLS;
					Mix_PlayChannel(-1, chooseSound, 0);
				}
				if (keys[SDL_SCANCODE_KP_MINUS]){
					exit(-1);
				}
			}

			//Map menu choice select
			if (gameState == STATE_MAP_SELECT){
				if (keys[SDL_SCANCODE_1]){
					mapChoice = 0;
				}
				if (keys[SDL_SCANCODE_2]){
					mapChoice = 1;
				}
				if (keys[SDL_SCANCODE_3]){
					mapChoice = 2;
				}

				if (mapChoice == 0) {
					if (keys[SDL_SCANCODE_SPACE]){
						gameState = STATE_GAME_LEVEL;
						buildLevel("flag_conquest.txt");
						gameStatReset();
						Mix_PlayMusic(gameMusic, -1);
					}
				}

				if (mapChoice == 1) {
					if (keys[SDL_SCANCODE_SPACE]){
						gameState = STATE_GAME_LEVEL;
						buildLevel("king_of_the_hill.txt");
						gameStatReset();
						Mix_PlayMusic(gameMusic, -1);
					}
				}

				if (mapChoice == 2) {
					if (keys[SDL_SCANCODE_SPACE]){
						gameState = STATE_GAME_LEVEL;
						buildLevel("modern_art.txt");
						gameStatReset();
						Mix_PlayMusic(gameMusic, -1);
					}
				}

				if (keys[SDL_SCANCODE_ESCAPE]){
					gameState = STATE_MAIN_MENU;
				}
				if (keys[SDL_SCANCODE_KP_MINUS]){
					exit(-1);
				}
			}

			//Game player controls
			if (gameState == STATE_GAME_LEVEL){

				//Player 1 movement
				if (keys[SDL_SCANCODE_D]){
					player1.direction = 1;
					if (player1.collideBottom == true){
						player1.velocityX = 1.0f;
					}
					else { 
						player1.velocityX = 0.75f;
					}
				}
				else if (keys[SDL_SCANCODE_A]){
					player1.direction = -1;
					if (player1.collideBottom == true){
						player1.velocityX = -1.0f;
					}
					else {
						player1.velocityX = -0.75f;
					}
				}
				else { player1.velocityX = 0.0f; }

				if (keys[SDL_SCANCODE_W] && player1.velocityY == 0){
					player1.velocityY += 0.4f;
					Mix_PlayChannel(-1, jumpSound, 0);
				}

				//Player 1 attacks
				if (keys[SDL_SCANCODE_J] && p1energy >= 1 && p1Cooldown <= 0){
					shootShot(player1);
					Mix_PlayChannel(-1, shotSound, 0);
					p1energy -= 1;
				}
				if (keys[SDL_SCANCODE_I] && p1energy >= 3 && p1Cooldown <= 0){
					shootLaser(player1);
					Mix_PlayChannel(-1, laserSound, 0);
					p1energy -= 3;
				}
				if (keys[SDL_SCANCODE_L] && p1energy >= 6 && p1Cooldown <= 0){
					shootBoom(player1);
					Mix_PlayChannel(-1, boomSound, 0);
					p1energy -= 6;
				}
				if (keys[SDL_SCANCODE_P] && p1energy >= 10 && p1Cooldown <= 0){
					selfDestruct(player1);
					Mix_PlayChannel(-1, bigsplosionSound, 0);
					p1health = 0;
				}

				//Player 2 movement
				if (keys[SDL_SCANCODE_RIGHT]){
					player2.direction = 1;
					if (player2.collideBottom == true){
						player2.velocityX = 1.0f;
					}
					else {
						player2.velocityX = 0.75f;
					}
				}
				else if (keys[SDL_SCANCODE_LEFT]){
					player2.direction = -1;
					if (player2.collideBottom == true){
						player2.velocityX = -1.0f;
					}
					else {
						player2.velocityX = -0.75f;
					}
				}
				else { player2.velocityX = 0.0f; }

				if (keys[SDL_SCANCODE_UP] && player2.velocityY == 0){
					player2.velocityY += 0.4f;
					Mix_PlayChannel(-1, jumpSound, 0);
				}

				//Player 2 attacks
				if (keys[SDL_SCANCODE_KP_4] && p2energy > 1 && p2Cooldown <= 0){
					shootShot(player2);
					Mix_PlayChannel(-1, shotSound, 0);
					p2energy -= 1;
				}
				if (keys[SDL_SCANCODE_KP_8] && p2energy > 3 && p2Cooldown <= 0){
					shootLaser(player2);
					Mix_PlayChannel(-1, laserSound, 0);
					p2energy -= 3;
				}
				if (keys[SDL_SCANCODE_KP_6] && p2energy > 6 && p2Cooldown <= 0){
					shootBoom(player2);
					Mix_PlayChannel(-1, boomSound, 0);
					p2energy -= 6;
				}
				if (keys[SDL_SCANCODE_KP_PLUS] && p2energy >= 10 && p2Cooldown <= 0){
					selfDestruct(player2);
					Mix_PlayChannel(-1, bigsplosionSound, 0);
					p2health = 0;
				}

				player1.x += player1.velocityX * FIXED_TIMESTEP;
				player1.y += player1.velocityY * FIXED_TIMESTEP;

				player2.x += player2.velocityX * FIXED_TIMESTEP;
				player2.y += player2.velocityY * FIXED_TIMESTEP;

				if (player1.velocityX != 0){
					p1walkCycler += 0.8;
					if (p1walkCycler > 2){
						if (p1walkState == WS_ONE) p1walkState = WS_TWO;
						else if (p1walkState == WS_TWO) p1walkState = WS_THREE;
						else if (p1walkState == WS_THREE) p1walkState = WS_FOUR;
						else if (p1walkState == WS_FOUR) p1walkState = WS_ONE;
						p1walkCycler = 0;
					}
				}
				else {
					p1walkCycler = 0;
					p1walkState = 0;
				}

				if (player2.velocityX != 0){
					p2walkCycler += 0.8;
					if (p2walkCycler > 2){
						if (p2walkState == WS_ONE) p2walkState = WS_TWO;
						else if (p2walkState == WS_TWO) p2walkState = WS_THREE;
						else if (p2walkState == WS_THREE) p2walkState = WS_FOUR;
						else if (p2walkState == WS_FOUR) p2walkState = WS_ONE;
						p2walkCycler = 0;
					}
				}
				else {
					p2walkCycler = 0;
					p2walkState = 0;
				}

				if (keys[SDL_SCANCODE_KP_MINUS]){
					exit(-1);
				}
			}

			//Escape from game over screen
			if (gameState == STATE_GAME_OVER){
				if (keys[SDL_SCANCODE_ESCAPE]){
					gameStatReset();
					flagVector.clear();
					bulletVector.clear();
					explosionVector.clear();
					gameState = STATE_START;
				}
				if (keys[SDL_SCANCODE_KP_MINUS]){
					exit(-1);
				}
			}
		}

		timeLeftOver = fixedElapsed;

		glClearColor(0.1f, 0.2f, 0.7f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		//Draw start screen
		if (gameState == STATE_START){
			modelMatrix.identity();
			modelMatrix.Translate(-0.9, 0.7, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "FLAG CAPPERS", 0.25, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-0.6, -0.1, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Press Space to start", 0.15, -0.08);
		}

		//Draw main menu screen
		if (gameState == STATE_MAIN_MENU){
			modelMatrix.identity();
			modelMatrix.Translate(-0.9, 0.7, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "FLAG CAPPERS", 0.25, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-0.3, 0.2, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Play Game", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-0.6, -0.1, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Rules and Controls", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-0.45, -0.5, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "W and S to navigate", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-0.35, -0.6, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "ENTER to select", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1, -0.85, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Hit NUMPAD '-' on any screen to close the game", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-1.0, 0.2, 0);
			modelMatrix.Scale(-7.0, 9.0, 1);
			program.setModelMatrix(modelMatrix);
			decoFlag1.sprite = SheetSprite(platformerSheet, 312, 30, 30);
			decoFlag1.draw(program);

			modelMatrix.identity();
			modelMatrix.Translate(-1.0, -0.4, 0);
			modelMatrix.Scale(8.0, 8.0, 1);
			program.setModelMatrix(modelMatrix);
			player1.draw(program);

			modelMatrix.identity();
			modelMatrix.Translate(1.0, 0.2, 0);
			modelMatrix.Scale(7.0, 9.0, 1);
			program.setModelMatrix(modelMatrix);
			decoFlag2.sprite = SheetSprite(platformerSheet, 313, 30, 30);
			decoFlag2.draw(program);

			modelMatrix.identity();
			modelMatrix.Translate(1.0, -0.4, 0);
			modelMatrix.Scale(-8.0, 8.0, 1);
			program.setModelMatrix(modelMatrix);
			player2.draw(program);

			if (menuChoice == LEVEL_SELECT){
			modelMatrix.identity();
			modelMatrix.Translate(-0.7, 0.2, 0);
			program.setModelMatrix(modelMatrix);
			indicator.draw(program);
			}
			if (menuChoice == RULES_CONTROLS){
				modelMatrix.identity();
				modelMatrix.Translate(-0.7,-0.1, 0);
				program.setModelMatrix(modelMatrix);
				indicator.draw(program);
			}

		}

		//Draw map select screen
		if (gameState == STATE_MAP_SELECT){
			modelMatrix.identity();
			modelMatrix.Translate(-0.75, 0.7, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "SELECT MAP", 0.25, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-0.4, 0.3, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Flag Conquest", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-0.5, 0.0, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "King of the Hill", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-0.3, -0.3, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Modern Art", 0.15, -0.08);
			
			modelMatrix.identity();
			modelMatrix.Translate(-1.0, -0.6, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "USE 1,2,3 KEYS TO SELECT MAP", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.0, -0.8, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "HIT SPACE TO START GAME", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.0, 0.0, 0);
			modelMatrix.Scale(-7.0, 9.0, 1);
			program.setModelMatrix(modelMatrix);
			decoFlag1.sprite = SheetSprite(platformerSheet, 312, 30, 30);
			decoFlag1.draw(program);

			modelMatrix.identity();
			modelMatrix.Translate(1.0, 0.0, 0);
			modelMatrix.Scale(7.0, 9.0, 1);
			program.setModelMatrix(modelMatrix);
			decoFlag2.sprite = SheetSprite(platformerSheet, 313, 30, 30);
			decoFlag2.draw(program);

			if (mapChoice == MAP_ONE){
				modelMatrix.identity();
				modelMatrix.Translate(-0.7, 0.3, 0);
				program.setModelMatrix(modelMatrix);
				indicator.draw(program);
			}
			if (mapChoice == MAP_TWO){
				modelMatrix.identity();
				modelMatrix.Translate(-0.7, 0.0, 0);
				program.setModelMatrix(modelMatrix);
				indicator.draw(program);
			}
			if (mapChoice == MAP_THREE){
				modelMatrix.identity();
				modelMatrix.Translate(-0.7, -0.3, 0);
				program.setModelMatrix(modelMatrix);
				indicator.draw(program);
			}
		}

		//Draw controls screen
		if (gameState == STATE_CONTROLS){
			modelMatrix.identity();
			modelMatrix.Translate(-0.5, 0.8, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "CONTROLS", 0.2, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-0.5, 0.6, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Player 1     Player 2", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1, 0.45, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Left     A            LEFT", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1, 0.30, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Right    D            RIGHT", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1, 0.15, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Jump     W            UP", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1, 0.0, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Shoot    J            NUM_4", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1, -0.15, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Laser    I            NUM_8", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1,-0.30, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "BoomShot L            NUM_6", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1,-0.45, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Explode  P            NUM_+", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1, -0.65, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "HIT SPACE TO SEE RULES", 0.15, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.1, -0.85, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "HIT ESCAPE TO RETURN TO MENU", 0.15, -0.08);
		}

		//Draw rules screen
		if (gameState == STATE_RULES){
			modelMatrix.identity();
			modelMatrix.Translate(-0.25, 0.8, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "RULES", 0.2, -0.08);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, 0.6, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Players fight and capture flags to earn points", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, 0.5, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Killing a player earns 10 points", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, 0.4, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Having a flag earns 1 point a second", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, 0.3, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Stand by a flag for 3 seconds to capture it", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, 0.2, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "The first player to reach 500 points wins", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, 0.0, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Players have a max of 10 health and energy", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, -0.1, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Players use up energy to attack", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, -0.2, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Players build up energy over time", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, -0.3, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "The basic shot uses 1 energy and does 1 damage", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2,-0.4, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "The laser uses 3 energy and does 2 damage", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, -0.5, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "The boomshot uses 6 energy and does 5 damage", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, -0.6, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Exploding uses 10 energy and does 8 damage", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, -0.7, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "(It will also kill the player so be careful)", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, -0.8, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "HIT ENTER TO RETURN TO CONTROLS", 0.125, -0.07);

			modelMatrix.identity();
			modelMatrix.Translate(-1.2, -0.9, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "HIT ESCAPE TO RETURN TO MENU", 0.125, -0.07);
		}

		//Draw game map
		if (gameState == STATE_GAME_LEVEL){
			modelMatrix.identity();
			modelMatrix.Translate(-1.33, 1.0, 0); //Centers the map
			program.setModelMatrix(modelMatrix);

			glBindTexture(GL_TEXTURE_2D, platformerSheet);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			vector<float> vertexData;
			vector<float> texCoordData;
			for (int y = 0; y < mapHeight; y++) {
				for (int x = 0; x < mapWidth; x++) {
					if (levelData[y][x] > 0){
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
			glUseProgram(program.programID);

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
			glEnableVertexAttribArray(program.positionAttribute);

			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
			glEnableVertexAttribArray(program.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);

			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);

			//Add shake effect
			viewMatrix.identity();
			if (explosionVector.size() > 0){
				int latest = explosionVector.size() - 1;
				float shakeAmount = explosionVector[latest].lifeTime;
				int adjustedShake = round(shakeAmount*100);
				if (adjustedShake % 2 == 0){
					viewMatrix.Translate(shakeAmount/4, 0, 0);
				}
				else { viewMatrix.Translate(-shakeAmount/4, 0, 0); }
			}
			program.setViewMatrix(viewMatrix);

			//Draw flags
			for (int i = 0; i < flagVector.size(); ++i){
				modelMatrix.identity();
				modelMatrix.Translate(flagVector[i].x, flagVector[i].y, 0.0);
				program.setModelMatrix(modelMatrix);
				flagVector[i].draw(program);
			}

			//Draw bullets
			for (int i = 0; i < bulletVector.size(); ++i){
				modelMatrix.identity();
				modelMatrix.Translate(bulletVector[i].x, bulletVector[i].y, 0.0);
				program.setModelMatrix(modelMatrix);
				bulletVector[i].draw(program);
			}

			//Draw explosions
			for (int i = 0; i < explosionVector.size(); ++i){
				modelMatrix.identity();
				modelMatrix.Translate(explosionVector[i].x, explosionVector[i].y, 0.0);
				program.setModelMatrix(modelMatrix);
				explosionVector[i].draw(program);
			}

			//Draw player 1
			modelMatrix.identity();
			modelMatrix.Translate(player1.x, player1.y, 0.0);
			modelMatrix.Scale(player1.direction, 1, 1);
			program.setModelMatrix(modelMatrix);
			if (p1walkState == WS_ONE) player1.sprite = SheetSprite(platformerSheet, 88, 30, 30);
			else if (p1walkState == WS_TWO) player1.sprite = SheetSprite(platformerSheet, 89, 30, 30);
			else if (p1walkState == WS_THREE) player1.sprite = SheetSprite(platformerSheet, 86, 30, 30);
			else if (p1walkState == WS_FOUR) player1.sprite = SheetSprite(platformerSheet, 87, 30, 30);
			player1.draw(program);
			
			//Draw player 2
			modelMatrix.identity();
			modelMatrix.Translate(player2.x, player2.y, 0.0);
			modelMatrix.Scale(player2.direction, 1, 1);
			program.setModelMatrix(modelMatrix);
			if (p2walkState == WS_ONE) player2.sprite = SheetSprite(platformerSheet, 58, 30, 30);
			else if (p2walkState == WS_TWO) player2.sprite = SheetSprite(platformerSheet, 59, 30, 30);
			else if (p2walkState == WS_THREE) player2.sprite = SheetSprite(platformerSheet, 56, 30, 30);
			else if (p2walkState == WS_FOUR) player2.sprite = SheetSprite(platformerSheet, 57, 30, 30);
			player2.draw(program);
			
			//Draw player information
			modelMatrix.identity();
			modelMatrix.Translate(-1.29, -0.75, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Player 1", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-1.29, -0.85, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Health:", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-0.9, -0.85, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, to_string(p1health), 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-1.29, -0.95, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Energy:", 0.1, -0.05);

			int p1energyRound = round(p1energy);
			modelMatrix.identity();
			modelMatrix.Translate(-0.9, -0.95, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, to_string(p1energyRound), 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(0.02, -0.75, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Player 2", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(0.02, -0.85, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Health:", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(0.4, -0.85, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, to_string(p2health), 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(0.02, -0.95, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Energy:", 0.1, -0.05);

			int p2energyRound = round(p2energy);
			modelMatrix.identity();
			modelMatrix.Translate(0.4, -0.95, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, to_string(p2energyRound), 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-1.29, 0.92, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Player 1", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-1.29, 0.82, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Score:", 0.1, -0.05);

			int p1scoreRound = round(p1score);
			modelMatrix.identity();
			modelMatrix.Translate(-0.95, 0.82, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, to_string(p1scoreRound), 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-1.29, 0.72, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Kills:", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(-0.95, 0.72, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, to_string(p1kills), 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(0.02, 0.92, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Player 2", 0.1, -0.05);

			int p2scoreRound = round(p2score);
			modelMatrix.identity();
			modelMatrix.Translate(0.02, 0.82, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Score:", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(0.35, 0.82, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, to_string(p2scoreRound), 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(0.02, 0.72, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, "Kills:", 0.1, -0.05);

			modelMatrix.identity();
			modelMatrix.Translate(0.35, 0.72, 0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, fontSheet, to_string(p2kills), 0.1, -0.05);

		}

		//Draw game over screen
		if (gameState == STATE_GAME_OVER){

			viewMatrix.identity();
			program.setViewMatrix(viewMatrix);

			if (p1win == true){
				modelMatrix.identity();
				modelMatrix.Translate(-1.2, 0.7, 0);
				program.setModelMatrix(modelMatrix);
				DrawText(&program, fontSheet, "PLAYER 1 WINS!", 0.3, -0.1);

				modelMatrix.identity();
				modelMatrix.Scale(8.0, 8.0, 1);
				program.setModelMatrix(modelMatrix);
				player1.draw(program);

				modelMatrix.identity();
				modelMatrix.Translate(-0.7, 0, 0);
				modelMatrix.Scale(-8.0, 8.0, 1);
				program.setModelMatrix(modelMatrix);
				decoFlag1.sprite = SheetSprite(platformerSheet, 312, 30, 30);
				decoFlag1.draw(program);

				modelMatrix.identity();
				modelMatrix.Translate(0.7, 0, 0);
				modelMatrix.Scale(8.0, 8.0, 1);
				program.setModelMatrix(modelMatrix);
				decoFlag2.sprite = SheetSprite(platformerSheet, 312, 30, 30);
				decoFlag2.draw(program);

				modelMatrix.identity();
				modelMatrix.Translate(-1.0, -0.7, 0);
				program.setModelMatrix(modelMatrix);
				DrawText(&program, fontSheet, "HIT ESCAPE TO RETURN TO MENU", 0.15, -0.08);
			}

			else if (p2win == true){
				modelMatrix.identity();
				modelMatrix.Translate(-1.2, 0.7, 0);
				program.setModelMatrix(modelMatrix);
				DrawText(&program, fontSheet, "PLAYER 2 WINS!", 0.2, -0.08);

				modelMatrix.identity();
				modelMatrix.Scale(8.0, 8.0, 1);
				program.setModelMatrix(modelMatrix);
				player2.draw(program);

				modelMatrix.identity();
				modelMatrix.Translate(-0.7, 0, 0);
				modelMatrix.Scale(-8.0, 8.0, 1);
				program.setModelMatrix(modelMatrix);
				decoFlag1.sprite = SheetSprite(platformerSheet, 313, 30, 30);
				decoFlag1.draw(program);

				modelMatrix.identity();
				modelMatrix.Translate(0.7, 0, 0);
				modelMatrix.Scale(8.0, 8.0, 1);
				program.setModelMatrix(modelMatrix);
				decoFlag2.sprite = SheetSprite(platformerSheet, 313, 30, 30);
				decoFlag2.draw(program);

				modelMatrix.identity();
				modelMatrix.Translate(-1.0, -0.7, 0);
				program.setModelMatrix(modelMatrix);
				DrawText(&program, fontSheet, "HIT ESCAPE TO RETURN TO MENU", 0.15, -0.08);
			}
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
