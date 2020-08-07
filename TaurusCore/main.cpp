#include "RenderSystem/Renderer.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[])
{
	Renderer *pRenderer = new Renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
	bool isRunning = true;
	while (isRunning)
	{
		isRunning = pRenderer->UpdateFrame();
	}
	delete pRenderer;

	return 0;
}