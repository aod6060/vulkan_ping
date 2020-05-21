#pragma once

#include "sys.h"


class AppGame : public IApp
{
private:
	//VulkanContext* context;

public:
	virtual void init();
	virtual void update(float delta);
	virtual void render();
	virtual void release();
};