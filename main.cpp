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

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 640, 360);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint ufoTexture = LoadTexture("ufoBlue.png");
	GLuint shipTexture = LoadTexture("playerShip1_orange.png");
	GLuint meteorTexture = LoadTexture("meteorGrey_big4.png");
	GLuint meteorTwoTexture = LoadTexture("meteorBrown_big2.png");
	GLuint laserTexture = LoadTexture("laserBlue01.png");

	float lastFrameTicks = 0.0f;

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		float ticks = (float)SDL_GetTicks() / 1500.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		//UFO
		glBindTexture(GL_TEXTURE_2D, ufoTexture);

		float uVertices[] = { -0.5, 0.75, 0.5, 0.75, 0.5, 1.75, -0.5, 0.75, 0.5, 1.75, -0.5, 1.75 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, uVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float uTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, uTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Ship
		glBindTexture(GL_TEXTURE_2D, shipTexture);

		float sVertices[] = { -0.5, -2.0, 0.5, -2.0, 0.5, -1.0, -0.5, -2.0, 0.5, -1.0, -0.5, -1.0 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, sVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float sTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, sTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Laser, animated, sorta (does this count?)
		glBindTexture(GL_TEXTURE_2D, laserTexture);

		float lVertices[] = { -0.1, -1.0 + ticks, 0.1, -1.0 + ticks, 0.1, -0.5 + ticks, -0.1, -1.0 + ticks, 0.1, -0.5 + ticks, -0.1, -0.5 + ticks };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, lVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float lTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, lTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Meteor
		glBindTexture(GL_TEXTURE_2D, meteorTexture);

		float mVertices[] = { 2.0, -1.5, 3.5, -1.5, 3.5, 0.0, 2.0, -1.5, 3.5, 0.0, 2.0, 0.0 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, mVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float mTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, mTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Meteor two
		glBindTexture(GL_TEXTURE_2D, meteorTwoTexture);

		float mtwoVertices[] = {-1.5, 0.75, -2.25, 0.75, -2.25, 0.0, -1.5, 0.75, -2.25, 0.0, -1.5, 0.0 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, mtwoVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float mtwoTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, mtwoTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
