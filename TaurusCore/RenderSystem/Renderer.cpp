#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Renderer.h"

Renderer::Renderer(int width, int height):mWidth(width), mHeight(height)
{
	Init(width, height);
}

Renderer::~Renderer()
{
	Close();
}

bool Renderer::UpdateFrame()
{
	if (!isInited)
		return false;
	bool isRunning = true;
	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			isRunning = false;
		}
	}

	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(mRenderer);

	//SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	//ShowObjWireframe();

	SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	ShowObjShaded();

	//Update screen
	SDL_RenderPresent(mRenderer);
	return isRunning;
}

void Renderer::Init(int screenWidth, int ScreenHeight)
{
	zbuffer = new int[mWidth * mHeight];
	isInited = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		isInited = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}
		mWindow = SDL_CreateWindow("TaurusRender", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, ScreenHeight, SDL_WINDOW_SHOWN);
		if (mWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			isInited = false;
		}
		else
		{
			mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
			if (mRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				isInited = false;
			}
		}
	}
	
	tgaImage.read_tga_file("../Assets/brick3.tga");
}

void Renderer::Close()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	mWindow = NULL;
	mRenderer = NULL;
	SDL_Quit();
}


void Renderer::SDLDrawPixel(int x, int y)
{
	SDL_RenderDrawPoint(mRenderer, x, mHeight - 1 - y);
}

void Renderer::line(int x0, int y0, int x1, int y1)
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

Vec3f Renderer::barycentric(Vec3f* pts, Vec3f P)
{
	Vec3f u = cross(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]), Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));
	if (std::abs(u[2]) < 1e-2) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void Renderer::triangle(Vec3f* pts, Vec2f* uvs, float c)
{
	Vec2f bboxmin(mWidth - 1, mHeight - 1);
	Vec2f bboxmax(0, 0);
	Vec2f clamp(mWidth - 1, mHeight - 1);
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
			if (zbuffer[int(P.x + P.y * mWidth)] < P.z)
			{
				zbuffer[int(P.x + P.y * mWidth)] = P.z;
				Vec2f uv = uvs[0] * bc_screen[0] + uvs[1] * bc_screen[1] + uvs[2] * bc_screen[2];
				TGAColor tgaColor = tgaImage.get(uv[0] * tgaImage.get_width(), uv[1] * tgaImage.get_height()) * c;
				SDL_SetRenderDrawColor(mRenderer, tgaColor[2], tgaColor[1], tgaColor[0], 0xFF);
				SDLDrawPixel(P.x, P.y);
			}
		}
	}
}

void Renderer::ShowObjWireframe()
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

				int x0 = (attrib.vertices[3 * idx.vertex_index + 0]) * 8 + mWidth / 2.;
				int y0 = (attrib.vertices[3 * idx.vertex_index + 1]) * 8 + mHeight / 2.;
				int x1 = (attrib.vertices[3 * idx1.vertex_index + 0]) * 8 + mWidth / 2.;
				int y1 = (attrib.vertices[3 * idx1.vertex_index + 1]) * 8 + mHeight / 2.;
				line(x0, y0, x1, y1);
			}
			index_offset += fnum;
		}
	}
}

void Renderer::ShowObjShaded()
{
	//Depth Buffer
	for (int i = mWidth * mHeight; i--; zbuffer[i] = -std::numeric_limits<float>::max());

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
				int x0 = (attrib.vertices[3 * idx.vertex_index + 0]) * 8 + mWidth / 2.;
				int y0 = (attrib.vertices[3 * idx.vertex_index + 1]) * 8 + mHeight / 2;
				float z0 = attrib.vertices[3 * idx.vertex_index + 2];
				screen_coords[v] = Vec3f(x0, y0, z0);
				world_coords[v] = Vec3f(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2]);
				float u0 = attrib.texcoords[2 * idx.texcoord_index + 0];
				float v0 = attrib.texcoords[2 * idx.texcoord_index + 1];
				uv_coords[v] = Vec2f(u0 - (int)u0, v0 - (int)v0);
			}
			Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
			n.normalize();
			float intensity = (n * light_dir) * 0.5f + 0.5f;
			if (intensity > 0) {
				triangle(screen_coords, uv_coords, intensity);
			}
			index_offset += fnum;
		}
	}
}