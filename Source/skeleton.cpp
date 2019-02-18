#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <limits>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;

SDL_Event event;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE false

struct Intersection
{
	vec4 position;
	float distance;
	int triangleIndex;
};

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
vector<Triangle> testScene;
float focalLength = SCREEN_HEIGHT;
vec4 camera(0, 0, -3, 1);
mat4 cameraRotMatrix;
float yaw = 0.01;

vec4 lightPos(0, -0.5, -0.7, 1.0);
vec3 lightColor = 14.0f*vec3(1, 1, 1);

bool Update();
void Draw(screen* screen);
bool ClosestIntersection(vec4 start, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection);
void RayTracer(screen* screen);
void LookAt(const vec3 &from, const vec3 &to);
vec4 calcDir(int x, int y, vec4 u, vec4 v, vec4 w);
mat3 RotX(float angle);
mat3 RotY(float angle);
void Rotate(mat3 rotation);
void TranslateX(float amount);
void TranslateY(float amount);
void TranslateZ(float amount);
vec3 DirectLight(const Intersection& i);

int main(int argc, char* argv[]) {
	screen *screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE);
	LoadTestModel(testScene);
	vec4 matr1(1,0,0,0);
	vec4 matr2(0,1,0,0);
	vec4 matr3(0,0,1,0);
	vec4 matr4(0,0,0,1);
	cameraRotMatrix = mat4(matr1, matr2, matr3, matr4);

	while (Update())
	{
		Draw(screen);
		SDL_Renderframe(screen);
	}

	SDL_SaveImage(screen, "screenshot.bmp");

	KillSDL(screen);
	return 0;
}

/*Place your drawing here*/
void Draw(screen* screen) {
	/* Clear buffer */
	memset(screen->buffer, 0, screen->height*screen->width * sizeof(uint32_t));
	RayTracer(screen);
	/*vec3 colour(1.0, 0.0, 0.0);
	for (int i = 0; i < 1000; i++)
	{
		uint32_t x = rand() % screen->width;
		uint32_t y = rand() % screen->height;
		PutPixelSDL(screen, x, y, colour);
	}*/
}

/*Place updates of parameters here*/
bool Update() {
	static int t = SDL_GetTicks();
	/* Compute frame time */
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) {
			return false;
		} else if (e.type == SDL_KEYDOWN) {
			int key_code = e.key.keysym.sym;
			switch (key_code) {
			case SDLK_UP:
				/* Move camera forward */
				Rotate(RotX(yaw));
				break;
			case SDLK_DOWN:
				/* Move camera backwards */
				Rotate(RotX(-yaw));
				break;
			case SDLK_LEFT:
				/* Move camera left */
				Rotate(RotY(-yaw));
				//Rotate(RotY(-yaw));
				break;
			case SDLK_RIGHT:
				/* Move camera right */				
				Rotate(RotY(yaw));
				break;
			case SDLK_w: // Up
				lightPos.z += 0.5f;
				//TranslateZ(1);
				break;
			case SDLK_s: //Down
				lightPos.z -= 0.5f;
				//TranslateZ(-1);
				break;
			case SDLK_a: //Left
				lightPos.x -= 0.5f;
				//TranslateX(-1);
				break;
			case SDLK_d: //right
				lightPos.x += 0.5f;
				//TranslateX(1);
				break;
			case SDLK_e:
				lightPos.y -= 0.5f;
				break;
			case SDLK_q:
				lightPos.y += 0.5f;
				break;

			case SDLK_ESCAPE:
				/* Move camera quit */
				return false;
			}
		}
	}
	return true;
}



vec4 calcDir(int x, int y, vec4 u, vec4 v, vec4 w) {
	float dx = (x - SCREEN_WIDTH * 0.5)*u.x + (SCREEN_HEIGHT*0.5 - y)*v.x - focalLength;
	float dy = (x - SCREEN_WIDTH * 0.5)*u.y + (SCREEN_HEIGHT*0.5 - y)*v.y - focalLength;
	float dz = (x - SCREEN_WIDTH * 0.5)*u.z + (SCREEN_HEIGHT*0.5 - y)*v.z - focalLength;

	vec4 dir(dx, dy, dz, 1);
	return dir;
}

void RayTracer(screen* screen) {
	//vec4 right(cameraRotMatrix[0][0], cameraRotMatrix[0][1], cameraRotMatrix[0][2], 1);
	//vec4 down(cameraRotMatrix[1][0], cameraRotMatrix[1][1], cameraRotMatrix[1][2], 1);
	//vec4 forward(cameraRotMatrix[2][0], cameraRotMatrix[2][1], cameraRotMatrix[2][2], 1);
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			

			vec4 dir(x - SCREEN_WIDTH * 0.5, y - SCREEN_HEIGHT * 0.5, focalLength, 1);
			dir = cameraRotMatrix *dir;
			//vec4 dir = calcDir(x, y, right, down, forward);
			Intersection ClosestIntersectionItem;

			
			
			if (ClosestIntersection(camera, dir, testScene, ClosestIntersectionItem)) {
				vec3 color = DirectLight(ClosestIntersectionItem);
				PutPixelSDL(screen, x, y, color);
			} else {
				vec3 color(0, 0, 0);
				PutPixelSDL(screen, x, y, color);
			}

		}
	}

}

bool ClosestIntersection(vec4 start, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection) {
	closestIntersection.distance = std::numeric_limits<float>::max();
	bool result = false;
	float length = dir.length();
	vec4 direction(dir.x / length, dir.y / length, dir.z / length, 1);
	for (int i = 0; i < triangles.size(); i++) {
		Triangle triangle = triangles[i];
		vec4 v0 = triangle.v0;
		vec4 v1 = triangle.v1;
		vec4 v2 = triangle.v2;
		vec3 e1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
		vec3 e2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
		vec3 b = vec3(start.x - v0.x, start.y - v0.y, start.z - v0.z);
		mat3 A(-direction, e1, e2);
		vec3 x = glm::inverse(A) * b;
		if (x.x >= 0) {
			if (0 <= x.y && 0 <= x.z && (x.y + x.z) <= 1) {
				//intersection
				if (closestIntersection.distance > x.x) {
					closestIntersection.distance = x.x;
					closestIntersection.triangleIndex = i;
					closestIntersection.position = start + direction * x.x;
				}
				result = true;
			}
		}
	}
	return result;
	
}

vec3 DirectLight(const Intersection& i) {
	Triangle triangle = testScene[i.triangleIndex];
	vec3 P = (vec3)i.position;
	vec3 r = (((vec3)lightPos) - P);
	vec3 n = glm::normalize(triangle.normal);
	float angle = acosf(glm::dot(r, n) / (n.length()*r.length()));
	if (angle < (M_PI/2)) {
		float rLength = glm::length(r);
		r = glm::normalize(r);
		float dotProduct = glm::dot(n, r);
		if (dotProduct>0) {
			return (triangle.color)*lightColor * (float)((dotProduct) / (4 * M_PI*rLength*rLength));
		}		
	}

	return vec3(0,0,0);
}

void TranslateX(float amount) {
	cameraRotMatrix[3][0] += amount;
}

void TranslateY(float amount) {
	cameraRotMatrix[3][1] += amount;
}

void TranslateZ(float amount) {
	cameraRotMatrix[3][2] += amount;
}

void Rotate(mat3 rotation) {
	mat3 rotationExtract(cameraRotMatrix[0][0], cameraRotMatrix[0][1], cameraRotMatrix[0][2],
		cameraRotMatrix[1][0], cameraRotMatrix[1][1], cameraRotMatrix[1][2],
		cameraRotMatrix[2][0], cameraRotMatrix[2][1], cameraRotMatrix[2][2]);

	rotationExtract = rotation * rotationExtract;

	cameraRotMatrix[0][0] = rotationExtract[0][0];
	cameraRotMatrix[0][1] = rotationExtract[0][1];
	cameraRotMatrix[0][2] = rotationExtract[0][2];
	cameraRotMatrix[1][0] = rotationExtract[1][0];
	cameraRotMatrix[1][1] = rotationExtract[1][1];
	cameraRotMatrix[1][2] = rotationExtract[1][2];
	cameraRotMatrix[2][0] = rotationExtract[2][0];
	cameraRotMatrix[2][1] = rotationExtract[2][1];
	cameraRotMatrix[2][2] = rotationExtract[2][2];
}

mat3 RotX(float angle) {
	vec3 x0(1, 0, 0);
	vec3 x1(0, cosf(angle), sinf(angle));
	vec3 x2(0, -sinf(angle), cosf(angle));

	return mat3(x0, x1, x2);
}

mat3 RotY(float angle) {
	vec3 x0(cosf(angle), 0, -sinf(angle));
	vec3 x1(0, 1, 0);
	vec3 x2(sinf(angle), 0, cosf(angle));

	return mat3(x0, x1, x2);
}

void LookAt(const vec3 &from, const vec3 &to) {
	vec3 tmp(0, 1, 0);
	vec3 forward = glm::normalize(from - to);
	vec3 right = glm::cross(tmp, forward);
	vec3 up = glm::cross(forward, right);

	cameraRotMatrix[0][0] = right.x;
	cameraRotMatrix[0][1] = right.y;
	cameraRotMatrix[0][2] = right.z;
	cameraRotMatrix[0][3] = 0;

	cameraRotMatrix[1][0] = up.x;
	cameraRotMatrix[1][1] = up.y;
	cameraRotMatrix[1][2] = up.z;
	cameraRotMatrix[1][3] = 0;

	cameraRotMatrix[2][0] = forward.x;
	cameraRotMatrix[2][1] = forward.y;
	cameraRotMatrix[2][2] = forward.z;
	cameraRotMatrix[1][3] = 0;

	cameraRotMatrix[3][0] = from.x;
	cameraRotMatrix[3][1] = from.y;
	cameraRotMatrix[3][2] = from.z;
	cameraRotMatrix[3][3] = 1;
}