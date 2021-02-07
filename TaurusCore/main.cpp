#include "RenderSystem/Renderer.h"


int main(int argc, char* args[])
{
	Renderer *pRenderer = new Renderer();
	bool isRunning = true;
	while (isRunning)
	{
		isRunning = pRenderer->UpdateFrame();
	}
	delete pRenderer;

	return 0;
}