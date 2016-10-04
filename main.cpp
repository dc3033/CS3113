#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define PI 3.14159265

SDL_Window* displayWindow;

class Entity {
public:

	Entity::Entity(float x, float y, float rotation, int textureID, float width,
		float height, float x_stretch, float y_stretch, float speed, float direction_x, 
		float direction_y, int index, int spriteCountX, int spriteCountY)
		: x(x), y(y), rotation(rotation), textureID(textureID), width(width), height(height), 
		x_stretch(x_stretch), y_stretch(y_stretch), speed(speed), direction_x(direction_x), 
		direction_y(direction_y), index(index), spriteCountX(spriteCountX), spriteCountY(spriteCountY) {}

	void Draw(ShaderProgram program){

		float vertices[] = {
			x, y,
			x + width, y,
			x + width, y + height,
			x, y,
			x, y + height,
			x + width, y + height
		};

		if (index == -1){
			float texCoords[] = {
				0.0 * x_stretch, 1.0 * y_stretch,
				1.0 * x_stretch, 1.0 * y_stretch,
				1.0 * x_stretch, 0.0 * y_stretch,
				0.0 * x_stretch, 1.0 * y_stretch,
				0.0 * x_stretch, 0.0 * y_stretch,
				1.0 * x_stretch, 0.0 * y_stretch
			};
			glBindTexture(GL_TEXTURE_2D, textureID);

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);

			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(program.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);
		}
		else {
			float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
			float v = (float)(((int)index) / spriteCountX) / (float)spriteCountX;
			float spriteWidth = 1.0 / (float)spriteCountX;
			float spriteHeight = 1.0 / (float)spriteCountY;

			float texCoords[] = {
				u, v + spriteHeight,
				u + spriteWidth, v + spriteHeight,
				u + spriteWidth, v,
				u, v + spriteHeight,
				u, v,
				u + spriteWidth, v
			};
			glBindTexture(GL_TEXTURE_2D, textureID);

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);

			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(program.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);
		}
	};

	float x;
	float y;
	float rotation;

	int textureID;

	float width;
	float height;

	float x_stretch;
	float y_stretch;

	float speed;
	float direction_x;
	float direction_y;

	int index;
	int spriteCountX;
	int spriteCountY;
};

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

int getRandAngle() {
	return rand() % 360;
}

void wallBounce(float& angle){
	angle = 360 - angle;
}

void paddleBounce(float& angle){
	if (angle < 180) { angle = 180 - angle; }
	if (angle > 180) { angle = 540 - angle; }
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Boxy Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 640, 360);

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint wallTexture = LoadTexture("boxAlt.png");
	GLuint midTexture = LoadTexture("ladder_mid.png");
	GLuint playerTexture = LoadTexture("boxCoinAlt.png");
	GLuint pongTexture = LoadTexture("boxExplosive.png");
	GLuint fontSheet = LoadTexture("font1.png");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float lastFrameTicks = 0.0f;

	float angle = getRandAngle();
	int gameStart = 0;
	int p1Win = 0; 
	int p2Win = 0;

	Entity topWall(-3.55, 1.75, 0, wallTexture, 7.1, 0.25, 28, 1, 0, 0, 0, -1, 0, 0);
	Entity botWall(-3.55, -2.0, 0, wallTexture, 7.1, 0.25, 28, 1, 0, 0, 0, -1, 0, 0);
	Entity midLine(-0.125, -1.75, 0, midTexture, 0.25, 3.5, 1, 14, 0, 0, 0, -1, 0, 0);
	Entity player1(-3.55, -0.375, 0, playerTexture, 0.25, 0.75, 1, 3, 0, 0, 0, -1, 0, 0);
	Entity player2(3.30, -0.375, 0, playerTexture, 0.25, 0.75, 1, 3, 0, 0, 0, -1, 0, 0);
	Entity pongBox(-0.125, -0.125, 0, pongTexture, 0.25, 0.25, 1, 1, 1, angle, angle, -1, 0, 0);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && gameStart == 0) {
					gameStart = 1;
				}
			}
		}

		float ticks = (float)SDL_GetTicks() / 500.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClearColor(0.9f, 0.8f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Updating-----------------------------------------------------------------------------------------

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		if (gameStart == 1){
			if (keys[SDL_SCANCODE_W]) {
				player1.speed = 2 * elapsed;
				player1.direction_y = player1.direction_x = 90;
				player1.y += player1.speed * sin(player1.direction_y * PI / 180);
			}
			if (keys[SDL_SCANCODE_S]) {
				player1.speed = 2 * elapsed;
				player1.direction_y = player1.direction_x = 270;
				player1.y += player1.speed * sin(player1.direction_y * PI / 180);
			}
			if (keys[SDL_SCANCODE_UP]) {
				player2.speed = 2 * elapsed;
				player2.direction_y = player2.direction_x = 90;
				player2.y += player2.speed * sin(player2.direction_y * PI / 180);
			}
			if (keys[SDL_SCANCODE_DOWN]) {
				player2.speed = 2 * elapsed;
				player2.direction_y = player2.direction_x = 270;
				player2.y += player2.speed * sin(player2.direction_y * PI / 180);
			}
			pongBox.x += pongBox.speed * elapsed * cos(pongBox.direction_x * PI / 180);
			pongBox.y += pongBox.speed * elapsed * sin(pongBox.direction_y * PI / 180);		
		}

		//Collision detection--------------------------------------------------------------------------------
		
		if (pongBox.x > 3.3){
			p1Win = 1;
			gameStart = 0;
		}
		if (pongBox.x < -3.55){
			p2Win = 1;
			gameStart = 0;
		}

		if (player1.y > 1.0) player1.y = 1.0;
		if (player1.y < -1.75) player1.y = -1.75;
		if (player2.y > 1.0) player2.y = 1.0;
		if (player2.y < -1.75) player2.y = -1.75;

		if (pongBox.y > 1.5){
			wallBounce(angle);
			pongBox.direction_x = pongBox.direction_y = angle;
			pongBox.speed += 0.025;
		}
		if (pongBox.y < -1.75){
			wallBounce(angle);
			pongBox.direction_x = pongBox.direction_y = angle;
			pongBox.speed += 0.025;
		}
		if (pongBox.x < -3.30 && (pongBox.y + 0.25) > (player1.y) && (pongBox.y) < (player1.y + 0.75)){
			pongBox.x = -3.30;
			paddleBounce(angle);
			pongBox.direction_x = pongBox.direction_y = angle;
			pongBox.speed += 0.1;
		}
		if (pongBox.x > 3.05 && (pongBox.y + 0.25) >(player2.y) && (pongBox.y) < (player2.y + 0.75)){
			pongBox.x = 3.05;
			paddleBounce(angle);
			pongBox.direction_x = pongBox.direction_y = angle;
			pongBox.speed += 0.1;
		}

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glUseProgram(program.programID);

		//Rendering----------------------------------------------------------------------------------------

		topWall.Draw(program);
		botWall.Draw(program);
		midLine.Draw(program);
		player1.Draw(program);
		player2.Draw(program);
		pongBox.Draw(program);

		if (gameStart == 0 && p1Win == 0 && p2Win == 0) {
			Entity fP1(-2.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 80, 16, 16);
			Entity fR1(-2.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 82, 16, 16);
			Entity fE1(-1.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 69, 16, 16);
			Entity fS1(-1.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 83, 16, 16);
			Entity fS2(-0.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 83, 16, 16);
			fP1.Draw(program);
			fR1.Draw(program);
			fE1.Draw(program);
			fS1.Draw(program);
			fS2.Draw(program);

			Entity fS3(0.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 83, 16, 16);
			Entity fP2(0.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 80, 16, 16);
			Entity fA1(1.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 65, 16, 16);
			Entity fC1(1.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 67, 16, 16);
			Entity fE2(2.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 69, 16, 16);
			fS3.Draw(program);
			fP2.Draw(program);
			fA1.Draw(program);
			fC1.Draw(program);
			fE2.Draw(program);
		}

		if (gameStart == 0 && p1Win == 1) {
			Entity fP1(-1.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 80, 16, 16);
			Entity f1_1(-1.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 49, 16, 16);
			fP1.Draw(program);
			f1_1.Draw(program);

			Entity fW1(0.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 87, 16, 16);
			Entity fI1(0.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 73, 16, 16);
			Entity fN1(1.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 78, 16, 16);
			Entity fS1(1.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 83, 16, 16);
			fW1.Draw(program);
			fI1.Draw(program);
			fN1.Draw(program);
			fS1.Draw(program);
		}

		if (gameStart == 0 && p2Win == 1) {
			Entity fP1(-1.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 80, 16, 16);
			Entity f2_1(-1.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 50, 16, 16);
			fP1.Draw(program);
			f2_1.Draw(program);

			Entity fW1(0.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 87, 16, 16);
			Entity fI1(0.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 73, 16, 16);
			Entity fN1(1.25, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 78, 16, 16);
			Entity fS1(1.75, -0.25, 0, fontSheet, 0.5, 0.5, 0, 0, 0, 0, 0, 83, 16, 16);
			fW1.Draw(program);
			fI1.Draw(program);
			fN1.Draw(program);
			fS1.Draw(program);
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
