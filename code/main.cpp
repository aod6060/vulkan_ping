#include "sys.h"
#include "game.h"



int main(int argc, char** argv)
{
	AppGame game;

	app_init("Ping", 640, 480, &game);

	app_update();

	app_release();

	return 0;
}