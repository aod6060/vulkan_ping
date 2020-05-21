#include "game.h"


void AppGame::init()
{
	VulkanRender::getInstance()->init(true);
}

void AppGame::update(float delta)
{
}

void AppGame::render()
{
	VulkanRender::getInstance()->startFrame();
	VulkanRender::getInstance()->clear(glm::vec3(0.0f));
	
	VulkanRender::getInstance()->setProj(glm::ortho(0.0f, (float)app_getWidth(), (float)app_getHeight(), 0.0f));
	VulkanRender::getInstance()->setView(glm::mat4(1.0f));
	VulkanRender::getInstance()->setModel(
		glm::scale(glm::mat4(1.0f), glm::vec3(32.0f, 32.0f, 0.0f)) * 
		glm::translate(glm::mat4(1.0f), glm::vec3(32.0f, 32.0f, 0.0f))
	);
	VulkanRender::getInstance()->updateTransforms();

	VulkanRender::getInstance()->drawQuad();

	VulkanRender::getInstance()->endFrame();
}

void AppGame::release()
{
	VulkanRender::destroy();
}
