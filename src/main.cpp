#include "main.h"

Engine engine(80, 50);

int main() {
	while(!TCODConsole::isWindowClosed() && engine.gameStatus != Engine::QUIT) {
		engine.update();
		engine.render();
		TCODConsole::flush();
	}
	return 0;
}
