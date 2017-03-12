#include <stdio.h>
#include "main.h"

Destructible::Destructible(float _maxHp, float _defense, const char *_corpseName) : 
	maxHp(_maxHp), 
	hp(_maxHp),
	defense(_defense), 
	corpseName(_corpseName) {}

float Destructible::takeDamage(Actor *owner, float damage) {
	damage -= defense;
	if(damage > 0) {
		hp -= damage;
		if(hp <= 0) {
			die(owner);
		}
	}
	else {
		damage = 0;
	}
	return damage;
}

float Destructible::heal(float amount) {
	hp += amount;
	if(hp > maxHp) {
		amount -= hp - maxHp;
		hp = maxHp;
	}
	return amount;
}

void Destructible::die(Actor *owner) {
	// ur head asplode
	owner->ch = '%';
	owner->col = TCODColor::darkRed;
	owner->name = corpseName;
	owner->blocks = false;
	// draw corpse first so that it appears underneath living actors
	engine.sendToBack(owner);
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName) : Destructible(maxHp, defense, corpseName) {}

void MonsterDestructible::die(Actor *owner) {
	engine.gui->message(TCODColor::lightGrey, "%s is dead.\n", owner->name);
	Destructible::die(owner);
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char *corpseName) : Destructible(maxHp, defense, corpseName) {}

void PlayerDestructible::die(Actor *owner) {
	engine.gui->message(TCODColor::red, "You have died!\n");
	Destructible::die(owner);
	engine.gameStatus = Engine::DEFEAT;
}