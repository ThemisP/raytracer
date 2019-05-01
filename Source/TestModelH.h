#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>

// Used to describe a triangular surface:
class Triangle
{
public:
	glm::vec4 v0;
	glm::vec4 v1;
	glm::vec4 v2;
	glm::vec4 normal;
	glm::vec3 color;
	float reflectance;
	float ior;

	Triangle(glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec3 color)
		: v0(v0), v1(v1), v2(v2), color(color), reflectance(0), ior(0)
	{
		ComputeNormal();
	}

	Triangle( glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec3 color, float reflectance, float ior)
		: v0(v0), v1(v1), v2(v2), color(color), reflectance(reflectance), ior(ior)
	{
		ComputeNormal();
	}

	void ComputeNormal()
	{
	  glm::vec3 e1 = glm::vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
	  glm::vec3 e2 = glm::vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
	  glm::vec3 normal3 = glm::normalize( glm::cross( e2, e1 ) );
	  normal.x = normal3.x;
	  normal.y = normal3.y;
	  normal.z = normal3.z;
	  normal.w = 1.0;
	}
};

void LoadTerrainGeneration(std::vector<Triangle>& triangles, int xSize, int zSize) {
	using glm::vec3;
	using glm::vec4;

	// Defines colors:
	vec3 red(0.75f, 0.15f, 0.15f);
	vec3 yellow(0.75f, 0.75f, 0.15f);
	vec3 green(0.15f, 0.75f, 0.15f);
	vec3 cyan(0.15f, 0.75f, 0.75f);
	vec3 blue(0.15f, 0.15f, 0.75f);
	vec3 purple(0.75f, 0.15f, 0.75f);
	vec3 white(0.75f, 0.75f, 0.75f);

	triangles.clear();
	triangles.reserve(20);

	std::vector<vec4> vertices;
	vertices.clear();
	vertices.reserve((xSize + 1)*(zSize + 1));
	int i = 0;
	for (int z = 0; z <= zSize; z++) {
		for (int x = 0; x <= xSize; x++) {
			float random = ((float)rand() / (float)(RAND_MAX))*0.5 +1; 
			vertices[i] = vec4((x - xSize/2)*0.2, random, (z - zSize / 2)*0.2, 1);
			i++;
		}
	}

	int vert = 0;
	for (int z = 0; z < zSize; z++) {
		for (int x = 0; x < xSize; x++) {
			vec4 a = vertices[vert];
			vec4 b = vertices[vert + 1];
			vec4 c = vertices[vert + xSize + 1];

			triangles.push_back(Triangle(a, b, c, green, 0, 0));

			vec4 d = vertices[vert + 1];
			vec4 e = vertices[vert + xSize + 1];
			vec4 f = vertices[vert + xSize + 2];

			triangles.push_back(Triangle(d, e, f, red, 0, 0));
			vert++;
		}
		vert++;
	}

}

// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel( std::vector<Triangle>& triangles )
{
	using glm::vec3;
	using glm::vec4;

	// Defines colors:
	vec3 red(    0.75f, 0.15f, 0.15f );
	vec3 yellow( 0.75f, 0.75f, 0.15f );
	vec3 green(  0.15f, 0.75f, 0.15f );
	vec3 cyan(   0.15f, 0.75f, 0.75f );
	vec3 blue(   0.15f, 0.15f, 0.75f );
	vec3 purple( 0.75f, 0.15f, 0.75f );
	vec3 white(  0.75f, 0.75f, 0.75f );

	triangles.clear();
	triangles.reserve( 5*2*3 );

	// ---------------------------------------------------------------------------
	// Room

	float L = 555;			// Length of Cornell Box side.

	vec4 A(L,0,0,1);
	vec4 B(0,0,0,1);
	vec4 C(L,0,L,1);
	vec4 D(0,0,L,1);

	vec4 E(L,L,0,1);
	vec4 F(0,L,0,1);
	vec4 G(L,L,L,1);
	vec4 H(0,L,L,1);

	// Floor:
	triangles.push_back( Triangle( C, B, A, green ) );
	triangles.push_back( Triangle( C, D, B, green ) );

	// Left wall
	triangles.push_back( Triangle( A, E, C, purple ) );
	triangles.push_back( Triangle( C, E, G, purple ) );

	// Right wall
	triangles.push_back( Triangle( F, B, D, yellow ) );
	triangles.push_back( Triangle( H, F, D, yellow ) );

	// Ceiling
	triangles.push_back( Triangle( E, F, G, cyan ) );
	triangles.push_back( Triangle( F, H, G, cyan ) );

	// Back wall
	triangles.push_back( Triangle( G, D, C, white ) );
	triangles.push_back( Triangle( G, H, D, white ) );

	// ---------------------------------------------------------------------------
	// Short block

	A = vec4(290,0,114,1);
	B = vec4(130,0, 65,1);
	C = vec4(240,0,272,1);
	D = vec4( 82,0,225,1);
	       
	E = vec4(290,165,114,1);
	F = vec4(130,165, 65,1);
	G = vec4(240,165,272,1);
	H = vec4( 82,165,225,1);

	float redReflectiveness = 0.4f;
	float redIor = 0.0f;
	// Front
	triangles.push_back( Triangle(E,B,A,red, redReflectiveness, redIor) );
	triangles.push_back( Triangle(E,F,B,red, redReflectiveness, redIor) );

	// Front
	triangles.push_back( Triangle(F,D,B,red, redReflectiveness, redIor) );
	triangles.push_back( Triangle(F,H,D,red, redReflectiveness, redIor) );

	// BACK
	triangles.push_back( Triangle(H,C,D,red, redReflectiveness, redIor) );
	triangles.push_back( Triangle(H,G,C,red, redReflectiveness, redIor) );

	// LEFT
	triangles.push_back( Triangle(G,E,C,red, redReflectiveness, redIor) );
	triangles.push_back( Triangle(E,A,C,red, redReflectiveness, redIor) );

	// TOP
	triangles.push_back( Triangle(G,F,E,red, redReflectiveness, redIor) );
	triangles.push_back( Triangle(G,H,F,red, redReflectiveness, redIor) );

	// ---------------------------------------------------------------------------
	// Tall block

	A = vec4(423,0,247,1);
	B = vec4(265,0,296,1);
	C = vec4(472,0,406,1);
	D = vec4(314,0,456,1);
	       
	E = vec4(423,330,247,1);
	F = vec4(265,330,296,1);
	G = vec4(472,330,406,1);
	H = vec4(314,330,456,1);

	float blueReflectiveness = 0.8f;
	float blueIor = 0.0f;
	// Front
	triangles.push_back( Triangle(E,B,A,blue, blueReflectiveness, blueIor) );
	triangles.push_back( Triangle(E,F,B,blue, blueReflectiveness, blueIor) );

	// Front
	triangles.push_back( Triangle(F,D,B,blue, blueReflectiveness, blueIor) );
	triangles.push_back( Triangle(F,H,D,blue, blueReflectiveness, blueIor) );

	// BACK
	triangles.push_back( Triangle(H,C,D,blue, blueReflectiveness, blueIor) );
	triangles.push_back( Triangle(H,G,C,blue, blueReflectiveness, blueIor) );

	// LEFT
	triangles.push_back( Triangle(G,E,C,blue, blueReflectiveness, blueIor) );
	triangles.push_back( Triangle(E,A,C,blue, blueReflectiveness, blueIor) );

	// TOP
	triangles.push_back( Triangle(G,F,E,blue, blueReflectiveness, blueIor) );
	triangles.push_back( Triangle(G,H,F,blue, blueReflectiveness, blueIor) );


	// ----------------------------------------------
	// Scale to the volume [-1,1]^3

	for( size_t i=0; i<triangles.size(); ++i )
	{
		triangles[i].v0 *= 2/L;
		triangles[i].v1 *= 2/L;
		triangles[i].v2 *= 2/L;

		triangles[i].v0 -= vec4(1,1,1,1);
		triangles[i].v1 -= vec4(1,1,1,1);
		triangles[i].v2 -= vec4(1,1,1,1);

		triangles[i].v0.x *= -1;
		triangles[i].v1.x *= -1;
		triangles[i].v2.x *= -1;

		triangles[i].v0.y *= -1;
		triangles[i].v1.y *= -1;
		triangles[i].v2.y *= -1;

		triangles[i].v0.w = 1.0;
		triangles[i].v1.w = 1.0;
		triangles[i].v2.w = 1.0;
		
		triangles[i].ComputeNormal();
	}
}

#endif
