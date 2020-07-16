#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include "geometry.h"
#include <algorithm>
#include "tgaimage.h"

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

int* zbuffer = new int[SCREEN_WIDTH * SCREEN_HEIGHT];
TGAImage tgaImage = TGAImage();

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("TaurusRender", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
		}
	}

	return success;
}

void close()
{
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void SDLDrawPixel(int x, int y)
{
	SDL_RenderDrawPoint(gRenderer, x, SCREEN_HEIGHT - 1 - y);
}

void line(int x0, int y0, int x1, int y1)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy / float(dx));
	float error = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			SDLDrawPixel(y, x);
		}
		else {
			SDLDrawPixel(x, y);
		}
		error += derror;
		if (error > .5) {
			y += (y1 > y0 ? 1 : -1);
			error -= 1.;
		}
	}
}

Vec3f barycentric(Vec3f* pts, Vec3f P) {
	Vec3f u = cross(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]), Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));
	if (std::abs(u[2]) < 1e-2) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(Vec3f* pts, Vec2f* uvs, float c) {
	Vec2f bboxmin(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
	Vec2f bboxmax(0, 0);
	Vec2f clamp(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = pts[0][2] * bc_screen[0] + pts[1][2] * bc_screen[1] + pts[2][2] * bc_screen[2];
			if (zbuffer[int(P.x + P.y * SCREEN_WIDTH)] < P.z)
			{
				zbuffer[int(P.x + P.y * SCREEN_WIDTH)] = P.z;
				Vec2f uv = uvs[0] * bc_screen[0] + uvs[1] * bc_screen[1] + uvs[2] * bc_screen[2];
				TGAColor tgaColor = tgaImage.get(uv[0] * tgaImage.get_width(), uv[1] * tgaImage.get_height()) * c;
				SDL_SetRenderDrawColor(gRenderer, tgaColor[2], tgaColor[1], tgaColor[0], 0xFF);
				SDLDrawPixel(P.x, P.y);
			}
		}
	}
}

void ShowObjWireframe()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "../Assets/utah-teapot.obj", NULL, true);
	if (!ret) {
		printf("Failed to load/parse .obj.\n");
		return;
	}
	for (size_t i = 0; i < shapes.size(); i++) {
		size_t index_offset = 0;
		assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.material_ids.size());

		// For each face
		for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
			size_t fnum = shapes[i].mesh.num_face_vertices[f];
			// For each vertex in the face
			for (size_t v = 0; v < fnum; v++) {
				tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
				tinyobj::index_t idx1 = shapes[i].mesh.indices[index_offset + (v + 1) % fnum];

				int x0 = (attrib.vertices[3 * idx.vertex_index + 0]) * 8 + SCREEN_WIDTH /2.;
				int y0 = (attrib.vertices[3 * idx.vertex_index + 1]) * 8 + SCREEN_HEIGHT / 2.;
				int x1 = (attrib.vertices[3 * idx1.vertex_index + 0]) * 8 + SCREEN_WIDTH / 2.;
				int y1 = (attrib.vertices[3 * idx1.vertex_index + 1]) * 8 + SCREEN_HEIGHT / 2.;
				line(x0, y0, x1, y1);
			}
			index_offset += fnum;
		}
	}
}

void ShowObjShaded()
{
	//Depth Buffer
	for (int i = SCREEN_WIDTH * SCREEN_HEIGHT; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "../Assets/utah-teapot.obj", NULL, true);
	if (!ret) {
		printf("Failed to load/parse .obj.\n");
		return;
	}
	Vec3f light_dir(0, 0, -1);
	for (size_t i = 0; i < shapes.size(); i++) {
		size_t index_offset = 0;
		assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.material_ids.size());

		// For each face
		for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
			size_t fnum = shapes[i].mesh.num_face_vertices[f];
			Vec3f screen_coords[3];
			Vec3f world_coords[3];
			Vec2f uv_coords[3];
			// For each vertex in the face
			for (size_t v = 0; v < fnum; v++) {
				tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
				int x0 = (attrib.vertices[3 * idx.vertex_index + 0]) * 8 + SCREEN_WIDTH / 2.;
				int y0 = (attrib.vertices[3 * idx.vertex_index + 1]) * 8 + SCREEN_HEIGHT / 2;
				float z0 = attrib.vertices[3 * idx.vertex_index + 2];
				screen_coords[v] = Vec3f(x0, y0, z0);
				world_coords[v] = Vec3f(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2]);
				float u0 = attrib.texcoords[2 * idx.texcoord_index + 0];
				float v0 = attrib.texcoords[2 * idx.texcoord_index + 1];
				uv_coords[v] = Vec2f(u0 - (int)u0, v0 - (int)v0);
			}
			Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
			n.normalize();
			float intensity = n * light_dir;
			if (intensity > 0) {
				triangle(screen_coords, uv_coords, intensity);
			}
			index_offset += fnum;
		}
	}
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		bool quit = false;
		//Event handler
		SDL_Event e;
		//Texture
		tgaImage.read_tga_file("../Assets/brick3.tga");

		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
			}

			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(gRenderer);

			//SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			//ShowObjWireframe();

			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			ShowObjShaded();

			//Update screen
			SDL_RenderPresent(gRenderer);
		}
		
	}

	//Free resources and close SDL
	close();

	return 0;
}