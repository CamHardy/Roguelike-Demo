#include "main.h"

Container::Container(int _size) : size(_size) {}

Container::~Container() {
	inventory.clearAndDelete();
}

bool Container::add(Actor *actor) {
	if(size > 0 && inventory.size() >= size) {
		// inventory is full
		return false;
	}
	inventory.push(actor);
	return true;
}

void Container::remove(Actor *actor) {
	inventory.remove(actor);
}