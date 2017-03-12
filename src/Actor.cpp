#include "main.h"
#include <math.h>

Actor::Actor(int _x, int _y, int _ch, const char *_name, const TCODColor &_col) : 
	x(_x), 
	y(_y), 
	ch(_ch), 
	name(_name), 
	col(_col), 
	blocks(true), 
	attacker(NULL), 
	destructible(NULL), 
	ai(NULL),
	pickable(NULL),
	container(NULL) {}

Actor::~Actor() {
	if(attacker) delete attacker;
	if(destructible) delete destructible;
	if(ai) delete ai;
	if(pickable) delete pickable;
	if(container) delete container;
}

void Actor::update() {
	if(ai) ai->update(this);
}

void Actor::render() const {
	TCODConsole::root->setChar(x, y, ch);
	TCODConsole::root->setCharForeground(x, y, col);
}

float Actor::getDistance(int cx, int cy) const {
	int dx = x - cx;
	int dy = y - cy;
	return sqrtf(dx * dx + dy * dy);
}