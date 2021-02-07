#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Renderer.h"
#include <cmath>

Renderer::Renderer()
{
	mWidth = SCREEN_WIDTH;
	mHeight = SCREEN_HEIGHT;
	Init();
}

Renderer::~Renderer()
{
	Close();
	delete zBuffer;
	delete shadowBuffer;
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
		else if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.sym)
			{
				case SDLK_1:
					useWireFrame = false;
					break;
				case SDLK_2:
					useWireFrame = true;
					break;
				case SDLK_w:
					cameraPos.y += 1;
					break;
				case SDLK_s:
					cameraPos.y -= 1;
					break;
				case SDLK_a:
					cameraPos.x -=1;
					break;
				case SDLK_d:
					cameraPos.x += 1;
					break;
				case SDLK_q:
					modelYawAngle -= 10;
					break;
				case SDLK_e:
					modelYawAngle += 10;
					break;
				default:
					break;
			}
		}
	}

	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(mRenderer);
	ShowObjShaded();
	ShowTextTip();

	SDL_RenderPresent(mRenderer);
	return isRunning;
}

void Renderer::Init()
{
	zBuffer = new float[mWidth * mHeight];
	shadowBuffer = new float[mWidth * mHeight];
	isInited = true;
	useWireFrame = false;

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
		mWindow = SDL_CreateWindow("TaurusRender", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mWidth, mHeight, SDL_WINDOW_SHOWN);
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
	textTipImage.read_tga_file("../Assets/texttip.tga");
	cameraPos = Vector4(0, 1, -50, 1);
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
	SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
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

Vector3 Renderer::barycentric(Vector3* pts, Vector3 P)
{
	Vector3 u = Vector3(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]).cross(Vector3(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));
	if (std::abs(u[2]) < 1e-2) return Vector3(-1, 1, 1);
	return Vector3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void Renderer::triangle(Vector4* pts, IShader* shader, float* targetZBuffer)
{
	Vector3 screenPts[3];
	for (int i = 0; i < 3; i++)
	{
		screenPts[i] = Vector3(pts[i].x / pts[i].w, pts[i].y / pts[i].w, pts[i].z / pts[i].w);
		screenPts[i].x = (screenPts[i].x / 2 + 0.5) * mWidth;
		screenPts[i].y = (screenPts[i].y / 2 + 0.5) * mHeight;
		screenPts[i].z = (screenPts[i].z / 2 + 0.5);
		if (screenPts[i].x<0 || screenPts[i].x>mWidth || screenPts[i].y<0 || screenPts[i].y>mHeight || screenPts[i].z < 0 || screenPts[i].z>1)
			return;
	}

	if (useWireFrame)
	{
		line(screenPts[0].x, screenPts[0].y, screenPts[1].x, screenPts[1].y);
		line(screenPts[1].x, screenPts[1].y, screenPts[2].x, screenPts[2].y);
		line(screenPts[2].x, screenPts[2].y, screenPts[0].x, screenPts[0].y);
		return;
	}

	Vector2 bboxmin(mWidth - 1, mHeight - 1);
	Vector2 bboxmax(0, 0);
	Vector2 clamp(mWidth - 1, mHeight - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], screenPts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], screenPts[i][j]));
		}
	}
	Vector3 P;
	for (P.x = (int)bboxmin.x; P.x <= (int)bboxmax.x; P.x++) {
		for (P.y = (int)bboxmin.y; P.y <= (int)bboxmax.y; P.y++) {
			Vector3 bc_screen = barycentric(screenPts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = screenPts[0][2] * bc_screen[0] + screenPts[1][2] * bc_screen[1] + screenPts[2][2] * bc_screen[2];
			if (targetZBuffer[int(P.x + P.y * mWidth)] > P.z)
			{
				targetZBuffer[int(P.x + P.y * mWidth)] = P.z;
				if (!shader->depthPass)
				{
					TGAColor tgaColor = shader->fragment(bc_screen, mWidth, mHeight);
					SDL_SetRenderDrawColor(mRenderer, tgaColor[2], tgaColor[1], tgaColor[0], 0xFF);
					SDLDrawPixel(P.x, P.y);
				}
			}
		}
	}
}

void Renderer::ShowObjShaded()
{
	for (int i = mWidth * mHeight; i--; zBuffer[i] = shadowBuffer[i] = FLT_MAX);

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "../Assets/utah-teapot.obj", NULL, true);
	if (!ret) {
		printf("Failed to load/parse .obj.\n");
		return;
	}
	Vector3 lightDir(-40, 0, 0);
	Matrix4 modelMatrix = GetModelMatrix(modelYawAngle);
	Matrix4 mvpMatrix = GetProjectionMatrix(1.0 * mWidth / mHeight) * GetViewMatrix(cameraPos, 0) * modelMatrix;
	Matrix4 shadowMvpMatrix = GetProjectionMatrix(1.0 * mWidth / mHeight) * GetViewMatrix(Vector4(lightDir.x, lightDir.y, lightDir.z, 1), 90) * modelMatrix;
	for (size_t i = 0; i < shapes.size(); i++) {
		size_t index_offset = 0;
		assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.material_ids.size());
		SimpleShader shader(&mvpMatrix, &shadowMvpMatrix, &modelMatrix, &tgaImage, lightDir, shadowBuffer);
		DepthShader depthShader(&shadowMvpMatrix);
		// For each face
		for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
			size_t fnum = shapes[i].mesh.num_face_vertices[f];
			Vector4 modelPts;
			Vector4 clip_coords[3];
			Vector4 depth_clip_coords[3];
			Vector3 uv;
			Vector4 normal;
			Vector4 viewDir[3];
			// For each vertex in the face
			for (size_t v = 0; v < fnum; v++) {
				tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
				modelPts = Vector4(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2], 1);
				uv = Vector3(attrib.texcoords[2 * idx.texcoord_index + 0], attrib.texcoords[2 * idx.texcoord_index + 1], 0);
				normal = Vector4(attrib.normals[3 * idx.normal_index + 0], attrib.normals[3 * idx.normal_index + 1], attrib.normals[3 * idx.normal_index + 2],0);
				viewDir[v] = cameraPos - modelMatrix * modelPts;
				clip_coords[v] = shader.vertex(v, modelPts, uv, normal);
				depth_clip_coords[v] = depthShader.vertex(v, modelPts, uv, normal);
			}
			if(!useWireFrame)
				triangle(depth_clip_coords, &depthShader, shadowBuffer);

			if (shader.varying_normal.getColumn(0).dot(viewDir[0]) > 0 || shader.varying_normal.getColumn(1).dot(viewDir[1]) > 0 || shader.varying_normal.getColumn(2).dot(viewDir[2]) > 0)
				triangle(clip_coords, &shader, zBuffer);
			index_offset += fnum;
		}
	}
}

void Renderer::ShowTextTip()
{
	for (int i = 0; i < textTipImage.get_width(); i++)
	{
		for (int j = 0; j < textTipImage.get_height(); j++)
		{
			TGAColor tgaColor = textTipImage.get(i, textTipImage.get_height() - j);
			SDL_SetRenderDrawColor(mRenderer, tgaColor[2], tgaColor[1], tgaColor[0], 0xFF);
			SDLDrawPixel(i, j);
		}
	}
}
