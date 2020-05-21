#pragma once


class IApp
{
public:
	virtual void init() = 0;
	virtual void update(float delta) = 0;
	virtual void render() = 0;
	virtual void release() = 0;
};

void app_init(std::string cap, uint32_t width, uint32_t height, IApp* app);
void app_update();
void app_release();
std::string app_getCaption();
uint32_t app_getWidth();
uint32_t app_getHeight();
SDL_Window* app_getWindow();
void app_exit();
