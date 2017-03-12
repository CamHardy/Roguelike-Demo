#include "main.h"

Engine::Engine(int _sW, int _sH) : gameStatus(STARTUP), fovRadius(10), screenWidth(_sW), screenHeight(_sH) {
	TCODConsole::initRoot(screenWidth, screenHeight, "Something roguelike", false);
	player = new Actor(40, 25, 1, "player", TCODColor::white);
	player->destructible = new PlayerDestructible(30, 2, "your lumpy dead body");
	player->attacker = new Attacker(5);
	player->ai = new PlayerAi();
	player->container = new Container(26);
	actors.push(player);
	map = new Map(80, 43);
	gui = new Gui();
	gui->message(TCODColor::red, "Welcome to some roguelike game thing.\nYou're gonna die.\n");
}

Engine::~Engine() {
	actors.clearAndDelete();
	delete map;
	delete gui;
}

void Engine::sendToBack(Actor *actor) {
	actors.remove(actor);
	actors.insertBefore(actor, 0);
}

void Engine::update() {
	if(gameStatus == STARTUP)
		map->computeFov();
	gameStatus = IDLE;
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
	if(lastKey.vk == TCODK_ESCAPE)
		gameStatus = QUIT;
	player->update();
	if(gameStatus == NEW_TURN) {
		for(Actor **iterator = actors.begin(); iterator != actors.end(); iterator++) {
			Actor *actor = *iterator;
			if(actor != player) {
				actor->update();
			}
		}
	}
}

void Engine::render() {
	TCODConsole::root->clear();

	// draw the map
	map->render();

	// draw the actors
	for(Actor **iterator = actors.begin(); iterator != actors.end(); iterator++) {
		Actor *actor = *iterator;
		if(actor != player && map->isInFov(actor->x, actor->y)) {
			actor->render();
		}
	}

	player->render();
	// show the player's stats
	gui->render();
	//TCODConsole::root->print(1, screenHeight - 2, "HP: %d/%d", (int)player->destructible->hp, (int)player->destructible->maxHp);
}

Actor *Engine::getClosestMonster(int x, int y, float range) const {
	Actor *closest = NULL;
	float bestDistance = 1E6f;

	for(Actor **iterator = actors.begin(); iterator != actors.end(); iterator++) {
		Actor *actor = *iterator;
		if(actor != player && actor->destructible && !actor->destructible->isDead()) {
			float distance = actor->getDistance(x, y);
			if(distance < bestDistance && (distance <= range || range == 0.0f)) {
				bestDistance = distance;
				closest = actor;
			}
		}
	}
	return closest;
}

bool Engine::pickATile(int *x, int *y, float maxRange) {
	while(!TCODConsole::isWindowClosed()) {
		render();

		// highlight tiles within range
		for(int cx = 0; cx < map->width; cx++) {
			for(int cy = 0; cy < map->height; cy++) {
				if(map->isInFov(cx, cy) && (maxRange == 0 || player->getDistance(cx, cy) <= maxRange)) {
					TCODColor col = TCODConsole::root->getCharBackground(cx, cy);
					col = col * 1.2f;
					TCODConsole::root->setCharBackground(cx, cy, col);
				}
			}
		}
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);

		if(map->isInFov(mouse.cx, mouse.cy) && (maxRange == 0 || player->getDistance(mouse.cx, mouse.cy) <= maxRange)) {
			TCODConsole::root->setCharBackground(mouse.cx, mouse.cy, TCODColor::white);

			if(mouse.lbutton_pressed) {
				*x = mouse.cx;
				*y = mouse.cy;
				return true;
			}
		}
		if(mouse.rbutton_pressed || lastKey.vk != TCODK_NONE) {
			return false;
		}
		TCODConsole::flush();
	}
	return false;
}

Actor *Engine::getActor(int x, int y) const {
	for(Actor **iterator = actors.begin(); iterator != actors.end(); iterator++) {
		Actor *actor = *iterator;
		if(actor->x == x && actor->y == y && actor->destructible && !actor->destructible->isDead()) {
			return actor;
		}
	}
	return NULL;
}