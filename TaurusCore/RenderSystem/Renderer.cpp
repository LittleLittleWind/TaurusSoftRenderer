#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Renderer.h"
#include <cmath>

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
					cameraPos.y -= 1;
					break;
				case SDLK_s:
					cameraPos.y += 1;
					break;
				case SDLK_a:
					cameraPos.x +=1;
					break;
				case SDLK_d:
					cameraPos.x -= 1;
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

void Renderer::Init(int screenWidth, int ScreenHeight)
{
	zbuffer = new int[mWidth * mHeight];
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
	textTipImage.read_tga_file("../Assets/texttip.tga");
	cameraPos = Vector3(0, 1, -40);
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

void Renderer::triangle(Vector4* pts, float c, PhongShader *shader)
{
	Vector3 screenPts[3];
	for (int i = 0; i < 3; i++)
	{
		screenPts[i] = Vector3(pts[i].x / pts[i].w, pts[i].y / pts[i].w, pts[i].z / pts[i].w);
		screenPts[i].x = (screenPts[i].x / 2 + 0.5) * mWidth;
		screenPts[i].y = (screenPts[i].y / 2 + 0.5) * mHeight;
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
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vector3 bc_screen = barycentric(screenPts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = screenPts[0][2] * bc_screen[0] + screenPts[1][2] * bc_screen[1] + screenPts[2][2] * bc_screen[2];
			if (zbuffer[int(P.x + P.y * mWidth)] > P.z)
			{
				zbuffer[int(P.x + P.y * mWidth)] = P.z;
				TGAColor tgaColor = shader->fragment(bc_screen) * c;
				SDL_SetRenderDrawColor(mRenderer, tgaColor[2], tgaColor[1], tgaColor[0], 0xFF);
				SDLDrawPixel(P.x, P.y);
			}
		}
	}
}

void Renderer::ShowObjShaded()
{
	//Depth Buffer
	for (int i = mWidth * mHeight; i--; zbuffer[i] = (1 << 31) - 1);

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "../Assets/utah-teapot.obj", NULL, true);
	if (!ret) {
		printf("Failed to load/parse .obj.\n");
		return;
	}
	Vector3 light_dir(0, 0, -1);
	Matrix4 mvpMatrix = GetMVPMatrix();
	for (size_t i = 0; i < shapes.size(); i++) {
		size_t index_offset = 0;
		assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.material_ids.size());
		PhongShader shader(&mvpMatrix, &tgaImage);
		// For each face
		for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
			size_t fnum = shapes[i].mesh.num_face_vertices[f];
			Vector3 model_coords[3];
			Vector4 clip_coords[3];

			// For each vertex in the face
			for (size_t v = 0; v < fnum; v++) {
				tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
				Vector3 model_coord = Vector3(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2]);
				model_coords[v] = model_coord;
				float u0 = attrib.texcoords[2 * idx.texcoord_index + 0];
				float v0 = attrib.texcoords[2 * idx.texcoord_index + 1];
				shader.varying_uv.setColumn(v, Vector3(u0 - (int)u0, v0 - (int)v0, 0));
				clip_coords[v] = shader.vertex(model_coord);
			}

			Vector3 n = (model_coords[2] - model_coords[0]).cross(model_coords[1] - model_coords[0]);
			n.normalize();
			float intensity = (n.dot(light_dir)) * 0.5f + 0.5f;
			if (intensity > 0)
				triangle(clip_coords, intensity, &shader);
			
			index_offset += fnum;
		}
	}
}

Matrix4 Renderer::GetMVPMatrix()
{
	Matrix4 scaleM = Matrix4();
	scaleM.setColumn(0, Vector4(1, 0, 0, 0));
	scaleM.setColumn(1, Vector4(0, 1, 0, 0));
	scaleM.setColumn(2, Vector4(0, 0, 1, 0));
	scaleM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 rotationM = Matrix4();
	rotationM.setColumn(0, Vector4(1, 0, 0, 0));
	rotationM.setColumn(1, Vector4(0, 1, 0, 0));
	rotationM.setColumn(2, Vector4(0, 0, 1, 0));
	rotationM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 transformM = Matrix4();
	transformM.setColumn(0, Vector4(1, 0, 0, 0));
	transformM.setColumn(1, Vector4(0, 1, 0, 0));
	transformM.setColumn(2, Vector4(0, 0, 1, 0));
	transformM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 modelMatrix = transformM * rotationM * scaleM;

	Matrix4 minusM = Matrix4();
	minusM.setColumn(0, Vector4(1, 0, 0, 0));
	minusM.setColumn(1, Vector4(0, 1, 0, 0));
	minusM.setColumn(2, Vector4(0, 0, -1, 0));
	minusM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 cameraRotationM = Matrix4();
	cameraRotationM.setColumn(0, Vector4(1, 0, 0, 0));
	cameraRotationM.setColumn(1, Vector4(0, 1, 0, 0));
	cameraRotationM.setColumn(2, Vector4(0, 0, 1, 0));
	cameraRotationM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 cameraTransformM = Matrix4();
	cameraTransformM.setColumn(0, Vector4(1, 0, 0, 0));
	cameraTransformM.setColumn(1, Vector4(0, 1, 0, 0));
	cameraTransformM.setColumn(2, Vector4(0, 0, 1, 0));
	cameraTransformM.setColumn(3, Vector4(cameraPos.x, cameraPos.y, cameraPos.z, 1));
	Matrix4 viewMatrix = minusM * cameraRotationM.invert() * cameraTransformM.invert();

	float fovAngle = 60 * 3.14 / 180;
	float f = 1000, n = 0.3;
	Matrix4 projectionMatrix = Matrix4();
	projectionMatrix.setColumn(0, Vector4(1/ std::tan(fovAngle/2) * mHeight / mWidth, 0, 0, 0));
	projectionMatrix.setColumn(1, Vector4(0, 1/ std::tan(fovAngle / 2), 0, 0));
	projectionMatrix.setColumn(2, Vector4(0, 0, -(f + n) / (f - n), -1));
	projectionMatrix.setColumn(3, Vector4(0, 0, -2 * f * n / (f - n), 0));

	return projectionMatrix * viewMatrix * modelMatrix;
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
