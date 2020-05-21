#include "sys.h"

static std::string gcaption;
static uint32_t gwidth;
static uint32_t gheight;
static IApp* gapp = nullptr;
static bool grunning = true;

static SDL_Window* gwindow = nullptr;

void app_init(
	std::string cap, 
	uint32_t width, 
	uint32_t height, 
	IApp* app)
{
	gcaption = cap;
	gwidth = width;
	gheight = height;
	gapp = app;

	SDL_Init(SDL_INIT_EVERYTHING);

	gwindow = SDL_CreateWindow(
		gcaption.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		gwidth,
		gheight,
		SDL_WINDOW_SHOWN
	);

	input_init();

	if (gapp)
	{
		gapp->init();
	}
}

void app_update()
{
	SDL_Event e;

	uint32_t pre = SDL_GetTicks();
	uint32_t curr = 0;
	float delta = 0.0f;

	while (grunning)
	{
		curr = SDL_GetTicks();
		delta = (curr - pre) / 1000.0f;
		pre = curr;

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				grunning = false;
			}

			input_event(e);
		}

		if (gapp)
		{
			gapp->update(delta);
			gapp->render();
		}

		input_update();
	}
}

void app_release()
{
	if (gapp)
	{
		gapp->release();
	}

	SDL_DestroyWindow(gwindow);
	SDL_Quit();
}

std::string app_getCaption()
{
	return gcaption;
}

uint32_t app_getWidth()
{
	return gwidth;
}

uint32_t app_getHeight()
{
	return gheight;
}

SDL_Window* app_getWindow()
{
	return gwindow;
}

void app_exit()
{
	grunning = false;
}