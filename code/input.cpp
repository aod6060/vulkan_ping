#include "sys.h"


enum InputState
{
	IS_RELEASE = 0,
	IS_DOWN,
	IS_PRESS,
	IS_UP,
	IS_SIZE
};

// Keyboards
static InputState keys[KEY_SIZE];

static void onKeyDown(SDL_KeyboardEvent& e);
static void onKeyUp(SDL_KeyboardEvent& e);

void input_init()
{
	for (uint32_t i = 0; i < KEY_SIZE; i++)
	{
		keys[i] = IS_RELEASE;
	}
}

void input_event(SDL_Event& e)
{
	switch (e.type)
	{
	case SDL_KEYDOWN:
		onKeyDown(e.key);
		break;
	case SDL_KEYUP:
		onKeyUp(e.key);
		break;
	}
}

void input_update()
{
	for (uint32_t i = 0; i < KEY_SIZE; i++)
	{
		if (keys[i] == IS_DOWN)
		{
			keys[i] = IS_PRESS;
		}

		if (keys[i] == IS_UP)
		{
			keys[i] = IS_RELEASE;
		}
	}
}

bool input_isKeyRelease(const Keyboard& k)
{
	return keys[k] == IS_RELEASE;
}

bool input_isKeyDown(const Keyboard& k)
{
	return keys[k] == IS_DOWN;
}

bool input_isKeyPress(const Keyboard& k)
{
	return keys[k] == IS_PRESS;
}

bool input_isKeyUp(const Keyboard& k)
{
	return keys[k] == IS_UP;
}

static void onKeyDown(SDL_KeyboardEvent& e)
{
	if (keys[e.keysym.scancode] == IS_RELEASE)
	{
		keys[e.keysym.scancode] = IS_DOWN;
	}
}

static void onKeyUp(SDL_KeyboardEvent& e)
{
	if (keys[e.keysym.scancode] == IS_PRESS)
	{
		keys[e.keysym.scancode] = IS_UP;
	}
}
