#include "main.h"

bool Pickable::pick(Actor *owner, Actor *wearer) {
	if(wearer->container && wearer->container->add(owner)) {
		engine.actors.remove(owner);
		return true;
	}
	return false;
}

void Pickable::drop(Actor *owner, Actor *wearer) {
	if(wearer->container) {
		wearer->container->remove(owner);
		engine.actors.push(owner);
		owner->x = wearer->x;
		owner->y = wearer->y;
		engine.gui->message(TCODColor::lightGrey, "%s drops a %s.", wearer->name, owner->name);
	}
}

bool Pickable::use(Actor *owner, Actor *wearer) {
	if(wearer->container) {
		wearer->container->remove(owner);
		delete owner;
		return true;
	}
	return false;
}

Healer::Healer(float _amount) : amount(_amount) {}

bool Healer::use(Actor *owner, Actor *wearer) {
	if(wearer->destructible) {
		float amountHealed = wearer->destructible->heal(amount);
		if(amountHealed > 0) {
			return Pickable::use(owner, wearer);
		}
	}
	return false;
}

LightningBolt::LightningBolt(float _range, float _damage) : range(_range), damage(_damage) {}

bool LightningBolt::use(Actor *owner, Actor *wearer) {
	Actor *closestMonster = engine.getClosestMonster(wearer->x, wearer->y, range);
	if(!closestMonster) {
		engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to strike.");
		return false;
	}

	// hit closest monster for <damage> hit points
	engine.gui->message(TCODColor::lightBlue, "A lightning bolt strikes the %s with a loud thunder!\nThe damage is %g hit points.", closestMonster->name, damage);
	closestMonster->destructible->takeDamage(closestMonster, damage);
	return Pickable::use(owner, wearer);
}

Fireball::Fireball(float range, float damage) : LightningBolt(range, damage) {}

bool Fireball::use(Actor *owner, Actor *wearer) {
	engine.gui->message(TCODColor::cyan, "Left-click a target tile for the fireball, \nor right-click to cancel.");
	int x, y;
	if(!engine.pickATile(&x, &y)) {
		return false;
	}

	// burn everything in <range> (including player XD)
	engine.gui->message(TCODColor::orange, "The fireball explodes, burning everything within %g tiles!", range);
	for(Actor **iterator = engine.actors.begin(); iterator != engine.actors.end(); iterator++) {
		Actor *actor = *iterator;
		if(actor->destructible && !actor->destructible->isDead() && actor->getDistance(x, y) <= range) {
			engine.gui->message(TCODColor::orange, "The %s gets burned for %g hit points.", actor->name, damage);
			actor->destructible->takeDamage(actor, damage);
		}
	}
	return Pickable::use(owner, wearer);
}

Confuser::Confuser(int _nbTurns, float _range) : nbTurns(_nbTurns), range(_range) {}

bool Confuser::use(Actor *owner, Actor *wearer) {
	engine.gui->message(TCODColor::cyan, "Left-click an enemy to confuse it,\nor right-click to cancel.");
	int x, y;
	if(!engine.pickATile(&x, &y, range)) {
		return false;
	}
	Actor *actor = engine.getActor(x, y);
	if(!actor) {
		return false;
	}

	// confuse the monster for <nbTurns> turns
	Ai *confusedAi = new ConfusedMonsterAi(nbTurns, actor->ai);
	actor->ai = confusedAi;
	engine.gui->message(TCODColor::lightGreen, "The eyes of the %s look vacant,\nas he starts to stumble around!", actor->name);
	return Pickable::use(owner, wearer);
}