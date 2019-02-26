#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include "Settings.h"
#include <stdint.h>
#include <limits>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;

SDL_Event event;

//#define SCREEN_WIDTH 320
//#define SCREEN_HEIGHT 256
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 720
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
float yaw = 0.01f;

vec4 lightPos(0, -0.5, -0.7, 1.0);
vec3 lightColor = 14.0f*vec3(1, 1, 1);
vec3 indirectLight = 0.5f*vec3(1, 1, 1);

bool Update();
void Draw(screen* screen);
bool ClosestIntersection(vec4 start, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection, int = (-1));
void RayTracer(screen* screen);
vec3 RayTraceColor(float x, float y);
void LookAt(const vec3 &from, const vec3 &to);
vec4 calcDir(int x, int y, vec4 u, vec4 v, vec4 w);
mat3 RotX(float angle);
mat3 RotY(float angle);
void Rotate(mat3 rotation);
void TranslateX(float amount);
void TranslateY(float amount);
void TranslateZ(float amount);
vec3 DirectLight(const Intersection& i);
float determinantFind(vec3 a, vec3 b, vec3 c);

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
			case SDLK_i://up
				TranslateZ(10);
				break;
			case SDLK_k://down
				TranslateZ(-10);
				break;
			case SDLK_j://left
				TranslateX(10);
				break;
			case SDLK_l://right
				TranslateX(-10);
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
	float dx = (x - SCREEN_WIDTH * 0.5f)*u.x + (SCREEN_HEIGHT*0.5f - y)*v.x - focalLength;
	float dy = (x - SCREEN_WIDTH * 0.5f)*u.y + (SCREEN_HEIGHT*0.5f - y)*v.y - focalLength;
	float dz = (x - SCREEN_WIDTH * 0.5f)*u.z + (SCREEN_HEIGHT*0.5f - y)*v.z - focalLength;

	vec4 dir(dx, dy, dz, 1);
	return dir;
}

void RayTracer(screen* screen) {
	int anti_aliasing = Settings::anti_aliasing_level;

	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			if (Settings::anti_aliasing) {
				vec3 color(0, 0, 0);

				float step = 1 / ((float)anti_aliasing * 2);
				for (int x_sample = -anti_aliasing; x_sample <= anti_aliasing; x_sample++) {
					for (int y_sample = -anti_aliasing; y_sample <= anti_aliasing; y_sample++) {
						color += RayTraceColor(x + step * x_sample, y + step * y_sample);
					}
				}
				color = color / ((float)(anti_aliasing * 2 + 1)*(anti_aliasing * 2 + 1));
				//color = RayTraceColor(x ,y);
				PutPixelSDL(screen, x, y, color);
			} else {
				vec3 color = RayTraceColor(x, y);
				PutPixelSDL(screen, x, y, color);
			}

		}
	}

}

vec3 RayTraceColor(float x, float y) {
	vec4 dir(x - SCREEN_WIDTH * 0.5f, y - SCREEN_HEIGHT * 0.5f, focalLength, 1);
	dir = cameraRotMatrix * dir;
	Intersection ClosestIntersectionItem;



	if (ClosestIntersection(camera, dir, testScene, ClosestIntersectionItem)) {
		vec3 color = testScene[ClosestIntersectionItem.triangleIndex].color *
			(DirectLight(ClosestIntersectionItem) + indirectLight);
		return color;
	} else {
		return vec3(0, 0, 0);
	}
}

bool ClosestIntersection(vec4 start, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection, int exclude) {
	closestIntersection.distance = std::numeric_limits<float>::max();
	bool result = false;
	float length = glm::length(dir);
	vec4 direction(dir.x / length, dir.y / length, dir.z / length, 1);
	for (size_t i = 0; i < triangles.size(); i++) {
		if (i != exclude) {
			Triangle triangle = triangles[i];
			vec4 v0 = triangle.v0;
			vec4 v1 = triangle.v1;
			vec4 v2 = triangle.v2;
			vec3 e1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
			vec3 e2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
			vec3 b = vec3(start.x - v0.x, start.y - v0.y, start.z - v0.z);

			float det = glm::determinant(mat3(-direction, e1, e2));
			if (det != 0) {
				float t = glm::determinant(mat3(b, e1, e2))/det;
				if (t >= 0) {
					float u = glm::determinant(mat3(-direction, b, e2)) / det;
					float v = glm::determinant(mat3(-direction, e1, b)) / det;
					if (0 <= u && 0 <= v && (u + v) <= 1) {
						//intersection
						if (closestIntersection.distance > t) {
							closestIntersection.distance = t;
							closestIntersection.triangleIndex = i;
							closestIntersection.position = start + direction * t;
						}
						result = true;
					}
				}
			}
			
			
		}
	}
	return result;
	
}

float determinantFind(vec3 a,vec3 b,vec3 c) {
	return -glm::dot(glm::cross(a, c),b);
}

vec3 DirectLight(const Intersection& i) {
	Triangle triangle = testScene[i.triangleIndex];
	vec3 P = (vec3)i.position;
	vec3 r = (((vec3)lightPos)-P);
	vec3 n = glm::normalize(triangle.normal);
	float angle = acosf(glm::dot(r, n) / (n.length()*r.length()));
	Intersection intersection;
	float rLength = glm::length(r);
	if (ClosestIntersection(vec4(P,1)+vec4(n*0.0001f,1), vec4(r,1), testScene, intersection, i.triangleIndex)) {
		if (intersection.distance>0.f && intersection.distance < rLength) {
			return vec3(0, 0, 0);
		} 
	}
	if (angle < (M_PI / 2)) {
		r = glm::normalize(r);
		float dotProduct = glm::dot(n, r);
		if (dotProduct>0) {
			return lightColor * (float)((dotProduct) / (4 * M_PI*rLength*rLength));
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
