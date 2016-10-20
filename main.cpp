#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include "Vector3.h"
#include "SheetSprite.h"
#include "Entity.h"
#include <vector>
#include <algorithm>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing, float x, float y) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++){
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing)*i) + x, y + size,
			((size + spacing)*i) + x, y,
			((size + spacing)*i) + x + size, y + size,
			((size + spacing)*i) + x + size, y,
			((size + spacing)*i) + x + size, y + size,
			((size + spacing)*i) + x, y,
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

void shootEnemyBullet(std::vector<Entity>& enemyBullets, GLuint texture, float x, float y) {
	Entity newBullet;
	newBullet.sprite = SheetSprite(texture, 843.0f / 1024.0f, 789.0f / 1024.0f, 13.0f / 1024.0f, 57.0f / 1024.0f, 0.03, -3.0, 0.0); //laserRed14
	newBullet.position = Vector3(x + 0.085, y - 0.14, 0);
	newBullet.sprite.x = newBullet.position.x;
	newBullet.sprite.y = newBullet.position.y;
	newBullet.velocity = Vector3(0, -0.05, 0);
	enemyBullets.push_back(newBullet);
}

void shootPlayerBullet(std::vector<Entity>& playerBullets, GLuint texture, float x, float y) {
	Entity newBullet;
	newBullet.sprite = SheetSprite(texture, 843.0f / 1024.0f, 789.0f / 1024.0f, 13.0f / 1024.0f, 57.0f / 1024.0f, 0.03, -3.0, 0.0); //laserRed14
	newBullet.position = Vector3(x + 0.085, y + 0.2, 0);
	newBullet.sprite.x = newBullet.position.x;
	newBullet.sprite.y = newBullet.position.y;
	newBullet.velocity = Vector3(0, 0.1, 0);
	playerBullets.push_back(newBullet);
}

bool shouldRemoveBullet(Entity bullet) {
	if (bullet.alive == 0 || bullet.position.y > 2.7 || bullet.position.y < -2.9) {
		return true;
	}
	else { 
		return false;
	}
}

enum GameState {STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_WIN_GAME, STATE_LOSE_GAME};

int state = STATE_MAIN_MENU;

float lastFrameTicks = 0.0f;
float clock = 0.0f;
bool left = true;
bool turn = false;
float reload = 1.0f;
float enemyReload = 1.0f;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Spehss Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 640, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 480, 640);

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-2.0f, 2.0f, -2.66f, 2.66f, -1.0f, 1.0f);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	//load spritesheets and textures
	GLuint fontSheet = LoadTexture("font1.png");
	GLuint spaceSheet = LoadTexture("sheet.png");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//create bullet and player/enemy entity vectors
	std::vector<Entity> playerBullets;

	std::vector<Entity> enemyBullets;

	std::vector<Entity> entities;
	int i;

	//create player and enemy entities and store in entities vector
	Entity myEntity;
	myEntity.position = Vector3(-2.0, -2.2, 0.0);
	myEntity.sprite = SheetSprite(spaceSheet, 336.0f / 1024.0f, 309.0f / 1024.0f, 98.0f / 1024.0f, 75.0f / 1024.0f, 0.2, -2.0, -2.2); //playerShip3_orange
	entities.push_back(myEntity);
	for (i = 0; i < 11; ++i){
		myEntity.sprite = SheetSprite(spaceSheet, 518.0f / 1024.0f, 409.0f / 1024.0f, 82.0f / 1024.0f, 84.0f / 1024.0f, 0.2, -2.0, 0.6); //enemyBlue4
		myEntity.position = Vector3(-2.0 + i*0.3, 0.6, 0);
		myEntity.sprite.x = myEntity.position.x;
		myEntity.sprite.y = myEntity.position.y;
		entities.push_back(myEntity);
	}
	for (i = 0; i < 11; ++i){
		myEntity.sprite = SheetSprite(spaceSheet, 518.0f / 1024.0f, 409.0f / 1024.0f, 82.0f / 1024.0f, 84.0f / 1024.0f, 0.2, -2.0, 0.9); //enemyBlue4
		myEntity.position = Vector3(-2.0 + i*0.3, 0.9, 0);
		myEntity.sprite.x = myEntity.position.x;
		myEntity.sprite.y = myEntity.position.y;
		entities.push_back(myEntity);
	}
	for (i = 0; i < 11; ++i){
		myEntity.sprite = SheetSprite(spaceSheet, 518.0f / 1024.0f, 493.0f / 1024.0f, 82.0f / 1024.0f, 84.0f / 1024.0f, 0.2, -2.0, 1.2); //enemyGreen4
		myEntity.position = Vector3(-2.0 + i*0.3, 1.2, 0);
		myEntity.sprite.x = myEntity.position.x;
		myEntity.sprite.y = myEntity.position.y;
		entities.push_back(myEntity);
	}
	for (i = 0; i < 11; ++i){
		myEntity.sprite = SheetSprite(spaceSheet, 518.0f / 1024.0f, 493.0f / 1024.0f, 82.0f / 1024.0f, 84.0f / 1024.0f, 0.2, -2.0, 1.5); //enemyGreen4
		myEntity.position = Vector3(-2.0 + i*0.3, 1.5, 0);
		myEntity.sprite.x = myEntity.position.x;
		myEntity.sprite.y = myEntity.position.y;
		entities.push_back(myEntity);
	}
	for (i = 0; i < 11; ++i){
		myEntity.sprite = SheetSprite(spaceSheet, 518.0f / 1024.0f, 325.0f / 1024.0f, 82.0f / 1024.0f, 84.0f / 1024.0f, 0.2, -2.0, 1.8); //enemyBlack4
		myEntity.position = Vector3(-2.0 + i*0.3, 1.8, 0);
		myEntity.sprite.x = myEntity.position.x;
		myEntity.sprite.y = myEntity.position.y;
		entities.push_back(myEntity);
	}

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && state == STATE_MAIN_MENU){
					state = STATE_GAME_LEVEL;
					clock = 0.0f;
				}
			}
		}
		
		float ticks = (float)SDL_GetTicks() / 500.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		clock += elapsed / 2.0f;
		reload -= elapsed;
		enemyReload -= elapsed;

		glClearColor(0.0f, 0.0f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		//update movement==============================================================

		if (state == STATE_GAME_LEVEL){
			//player movement
			if (keys[SDL_SCANCODE_A]) {
				entities[0].position.x -= 0.025;
				entities[0].sprite.x = entities[0].position.x;
			}
			if (keys[SDL_SCANCODE_D]) {
				entities[0].position.x += 0.025;
				entities[0].sprite.x = entities[0].position.x;
			}

			//player bullet spawn
			if (keys[SDL_SCANCODE_SPACE] && reload <= 0.0) {
				shootPlayerBullet(playerBullets, spaceSheet, entities[0].position.x, entities[0].position.y);
				reload = 0.5f;
			}

			//player bullet movement
			for (int i = 0; i < playerBullets.size(); ++i){
				playerBullets[i].position.y += playerBullets[i].velocity.y;
				playerBullets[i].sprite.y = playerBullets[i].position.y;
			}

			//enemy movement
			int killcount = 0;
			for (int i = 1; i < 56; ++i){
				if (entities[i].alive == 0) {
					killcount += 1;
				}
			}

			if (killcount == 55) { state = STATE_WIN_GAME; }

			Vector3 enemyVelocity(0.005, 0, 0);
			if (killcount >= 10 && killcount < 20) {
				enemyVelocity.x = 0.01;
			}
			else if (killcount >= 20 && killcount < 30) {
				enemyVelocity.x = 0.015;
			}
			else if (killcount >= 30 && killcount < 40) {
				enemyVelocity.x = 0.02;
			}
			else if (killcount >= 40 && killcount < 50) {
				enemyVelocity.x = 0.025;
			}
			else if (killcount >= 50) {
				enemyVelocity.x = 0.035;
			}

			for (int i = 1; i < 56; ++i){
				if (entities[i].alive == 1 && (entities[i].sprite.x + entities[i].sprite.size) >= 2.0) {
					left = false;
					turn = true;
					break;
				}
				if (entities[i].alive == 1 && (entities[i].sprite.x - entities[i].sprite.size) <= -2.2) {
					left = true;
					turn = true;
					break;
				}
			}

			if (turn){
				for (int i = 1; i < 56; ++i){
					entities[i].position.y -= 0.2;
					entities[i].sprite.y = entities[i].position.y;
				}
				turn = false;
			}

			for (int i = 1; i < 56; ++i){
				if (left) {
					entities[i].position.x += enemyVelocity.x;
					entities[i].sprite.x = entities[i].position.x;
				}
				if (!left) {
					entities[i].position.x -= enemyVelocity.x;
					entities[i].sprite.x = entities[i].position.x;
				}
			}

			//enemy bullet spawn
			if (enemyBullets.size() < 4 && enemyReload <= 0){
				std::vector<int> canShoot;
				for (int i = 1; i < 12; ++i){
					for (int j = i; j < 56; j += 11){
						if (entities[j].alive == 1){
							canShoot.push_back(j);
							break;
						}
					}
				}
				int randomIndex = rand() % canShoot.size();
				int shooter = canShoot[randomIndex];
				shootEnemyBullet(enemyBullets, spaceSheet, entities[shooter].position.x, entities[shooter].position.y);
				enemyReload = 1.5f;
			}

			//enemy bullet movement
			for (int i = 0; i < enemyBullets.size(); ++i){
				enemyBullets[i].position.y += enemyBullets[i].velocity.y;
				enemyBullets[i].sprite.y = enemyBullets[i].position.y;
			}
		}

		//update collision=============================================================

		//player movement restriction
		if (state == STATE_GAME_LEVEL){
			if (entities[0].position.x <= -2.0) {
				entities[0].position.x = -2.0;
				entities[0].sprite.x = entities[0].position.x;
			}
			if (entities[0].position.x >= 1.8) {
				entities[0].position.x = 1.8;
				entities[0].sprite.x = entities[0].position.x;
			}

			//player bullet collides with enemy
			for (int i = 0; i < playerBullets.size(); ++i){
				for (int j = 1; j < 56; ++j){
					if (entities[j].alive == 1
						&& entities[j].sprite.x < (playerBullets[i].sprite.x + playerBullets[i].sprite.size)
						&& (entities[j].sprite.x + entities[j].sprite.size) > playerBullets[i].sprite.x
						&& entities[j].sprite.y < (playerBullets[i].sprite.y + (playerBullets[i].sprite.size * playerBullets[i].sprite.reverseAspect))
						&& (entities[j].sprite.y + (entities[j].sprite.size * entities[j].sprite.reverseAspect)) > playerBullets[i].sprite.y){
						playerBullets[i].alive = 0;
						entities[j].alive = 0;
					}
				}
			}
				
			//enemy bullet collides with player
			for (int i = 0; i < enemyBullets.size(); ++i){
				if (entities[0].sprite.x < (enemyBullets[i].sprite.x + enemyBullets[i].sprite.size)
					&& (entities[0].sprite.x + entities[0].sprite.size) > enemyBullets[i].sprite.x
					&& entities[0].sprite.y < (enemyBullets[i].sprite.y + (enemyBullets[i].sprite.size * enemyBullets[i].sprite.reverseAspect))
					&& (entities[0].sprite.y + (entities[0].sprite.size * entities[0].sprite.reverseAspect)) > enemyBullets[i].sprite.y){
					entities[0].alive = 0;
					state = STATE_LOSE_GAME;
				}
			}

			
			//enemy collides with player
			for (int i = 1; i < entities.size(); ++i){
				if (entities[i].alive == 1
					&& entities[0].sprite.x < (entities[i].sprite.x + entities[i].sprite.size)
					&& (entities[0].sprite.x + entities[0].sprite.size) > entities[i].sprite.x
					&& entities[0].sprite.y < (entities[i].sprite.y + (entities[i].sprite.size * entities[i].sprite.reverseAspect))
					&& (entities[0].sprite.y + (entities[0].sprite.size * entities[0].sprite.reverseAspect)) > entities[i].sprite.y){
					entities[0].alive = 0;
					state = STATE_LOSE_GAME;
				}
			}
			

			//erase bullets
			playerBullets.erase((std::remove_if(playerBullets.begin(), playerBullets.end(), shouldRemoveBullet)), playerBullets.end());
			enemyBullets.erase((std::remove_if(enemyBullets.begin(), enemyBullets.end(), shouldRemoveBullet)), enemyBullets.end());
		}
		//render========================================================================

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glUseProgram(program.programID);

		if (state == STATE_MAIN_MENU){
			DrawText(&program, fontSheet, "SCORE<PAY $4.99 TO UNLOCK>", 0.25, -0.1, -2.0, 2.4);
			DrawText(&program, fontSheet, "CREDIT 42", 0.25, -0.1, 0.5, -2.6);
			if (clock > 1) DrawText(&program, fontSheet, "SPEHSS", 0.5, -0.1, -1.3, 1.5);
			if (clock > 2)DrawText(&program, fontSheet, "INVADERS", 0.5, -0.1, -1.7, 1.1);
			if (clock > 3){
				DrawText(&program, fontSheet, "*SCORE ADVANCE TABLE*", 0.25, -0.1, -1.65, 0.6);
				DrawText(&program, fontSheet, "UPGRADE TO", 0.25, -0.1, -0.8, 0.3);
				DrawText(&program, fontSheet, "PREMIUM ACCOUNT", 0.25, -0.1, -1.2, 0.0);
				DrawText(&program, fontSheet, "TO UNLOCK", 0.25, -0.1, -0.8, -0.3);
				DrawText(&program, fontSheet, "THIS FEATURE", 0.25, -0.1, -1.0, -0.6);
			}
			if (clock > 4)DrawText(&program, fontSheet, "PRESS SPACE TO PLAY", 0.25, -0.1, -1.55, -1.2);
		}

		if (state == STATE_GAME_LEVEL) {
			DrawText(&program, fontSheet, "SCORE<PAY $4.99 TO UNLOCK>", 0.25, -0.1, -2.0, 2.4);
			DrawText(&program, fontSheet, "CREDIT 42", 0.25, -0.1, 0.5, -2.6);
			for (int i = 0; i < entities.size(); ++i) {
				if (entities[i].alive == 1){
					entities[i].sprite.Draw(program);
				}
			}
			for (int i = 0; i < playerBullets.size(); ++i){
				playerBullets[i].sprite.Draw(program);
			}
			for (int i = 0; i < enemyBullets.size(); ++i){
				enemyBullets[i].sprite.Draw(program);
			}
		}

		if (state == STATE_WIN_GAME) {
			DrawText(&program, fontSheet, "YOU'RE WINNER", 0.4, -0.1, -2.0, 0.0);
			DrawText(&program, fontSheet, "SCORE<PAY $4.99 TO UNLOCK>", 0.25, -0.1, -2.0, 2.4);
			DrawText(&program, fontSheet, "CREDIT 42", 0.25, -0.1, 0.5, -2.6);
			for (int i = 0; i < entities.size(); ++i) {
				if (entities[i].alive == 1){
					entities[i].sprite.Draw(program);
				}
			}
			for (int i = 0; i < playerBullets.size(); ++i){
				playerBullets[i].sprite.Draw(program);
			}
			for (int i = 0; i < enemyBullets.size(); ++i){
				enemyBullets[i].sprite.Draw(program);
			}
		}
		
		if (state == STATE_LOSE_GAME) {
			DrawText(&program, fontSheet, "YOU DIED", 0.6, -0.1, -2.05, 0.0);
			DrawText(&program, fontSheet, "SCORE<PAY $4.99 TO UNLOCK>", 0.25, -0.1, -2.0, 2.4);
			DrawText(&program, fontSheet, "CREDIT 42", 0.25, -0.1, 0.5, -2.6);
			for (int i = 0; i < entities.size(); ++i) {
				if (entities[i].alive == 1){
					entities[i].sprite.Draw(program);
				}
			}
			for (int i = 0; i < playerBullets.size(); ++i){
				playerBullets[i].sprite.Draw(program);
			}
			for (int i = 0; i < enemyBullets.size(); ++i){
				enemyBullets[i].sprite.Draw(program);
			}
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
