#include "main.h"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int MAX_ROOM_ITEMS = 2;

class BspListener : public ITCODBspCallback {
private:
	Map &map;			// a map to dig
	int roomNum;		// room number
	int lastX, lastY;	// center of the last room
public:
	BspListener(Map &map) : map(map), roomNum(0) {}
	bool visitNode(TCODBsp *node, void *userData) {
		if(node->isLeaf()) {
			int x, y, w, h;

			// dig a room
			TCODRandom *rng = TCODRandom::getInstance();
			w = rng->getInt(ROOM_MIN_SIZE, node->w - 2);
			h = rng->getInt(ROOM_MIN_SIZE, node->h - 2);
			x = rng->getInt(node->x + 1, node->x + node->w - w - 1);
			y = rng->getInt(node->y + 1, node->y + node->h - h - 1);
			map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1);

			if(roomNum != 0) {
				// dig a corridor from last room
				map.dig(lastX, lastY, x + w / 2, lastY);
				map.dig(x + w / 2, lastY, x + w / 2, y + h /2);
			}

			lastX = x + w / 2;
			lastY = y + h / 2;
			roomNum++;
		}
		return true;
	}
};

Map::Map(int _width, int _height) : width(_width), height(_height) {
	tiles = new Tile[width * height];
	map = new TCODMap(width, height);
	TCODBsp bsp(0, 0, width, height);
	bsp.splitRecursive(NULL, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
	BspListener listener(*this);
	bsp.traverseInvertedLevelOrder(&listener, NULL);
}

Map::~Map() {
	delete [] tiles;
	delete map;
}

bool Map::isWall(int x, int y) const {
	return !map->isWalkable(x, y);
}

bool Map::canWalk(int x, int y) const {
	if(isWall(x, y)) {
		// this is a wall
		return false;
	}
	for(Actor **iterator = engine.actors.begin(); iterator != engine.actors.end(); iterator++) {
		Actor *actor = *iterator;
		if(actor->blocks && actor->x == x && actor->y == y) {
			// there is an actor there
			return false;
		}
	}
	return true;
}

bool Map::isExplored(int x, int y) const {
	return tiles[x + y * width].explored;
}

bool Map::isInFov(int x, int y) const {
	if(map->isInFov(x, y)) {
		tiles[x + y * width].explored = true;
		return true;
	}
	return false;
}

void Map::addMonster(int x, int y) {
	TCODRandom *rng = TCODRandom::getInstance();
	if(rng->getInt(0, 100) < 80) {
		// create an orc
		Actor *orc = new Actor(x, y, 'o', "orc", TCODColor::desaturatedGreen);
		orc->destructible = new MonsterDestructible(10, 0, "dead orc");
		orc->attacker = new Attacker(3);
		orc->ai = new MonsterAi();
		engine.actors.push(orc);
	}
	else {
		// create a troll
		Actor *troll = new Actor(x, y, 'T', "troll", TCODColor::darkerGreen);
		troll->destructible = new MonsterDestructible(16, 1, "troll carcass");
		troll->attacker = new Attacker(4);
		troll->ai = new MonsterAi();
		engine.actors.push(troll);
	}
}

void Map::addItem(int x, int y) {
	TCODRandom *rng = TCODRandom::getInstance();
	int dice = rng->getInt(0, 99);
	if(dice < 69) {
		// create a health potion
		Actor *healthPotion = new Actor(x, y, '!', "health potion", TCODColor::violet);
		healthPotion->blocks = false;
		healthPotion->pickable = new Healer(4);
		engine.actors.push(healthPotion);
	}
	else if(dice < 79) {
		// create a scroll of lightning bolt
		Actor *scrollOfLightningBolt = new Actor(x, y, '#', "scroll of lightning bolt", TCODColor::lightYellow);
		scrollOfLightningBolt->blocks = false;
		scrollOfLightningBolt->pickable = new LightningBolt(5, 20);
		engine.actors.push(scrollOfLightningBolt);
	}
	else if(dice < 89) {
		// create a scroll of fireball
		Actor *scrollOfFireball = new Actor(x, y, '#', "scroll of fireball", TCODColor::lightYellow);
		scrollOfFireball->blocks = false;
		scrollOfFireball->pickable = new Fireball(3, 12);
		engine.actors.push(scrollOfFireball);
	}
	else {
		// create a scroll of confusion
		Actor *scrollOfConfusion = new Actor(x, y, '#', "scroll of confusion", TCODColor::lightYellow);
		scrollOfConfusion->blocks = false;
		scrollOfConfusion->pickable = new Confuser(10, 8);
		engine.actors.push(scrollOfConfusion);
	}
}

void Map::computeFov() {
	map->computeFov(engine.player->x, engine.player->y, engine.fovRadius);
}

void Map::dig(int x1, int y1, int x2, int y2) {
	if(x2 < x1) {
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}
	if(y2 < y1) {
		int tmp = y2;
		y2 = y1;
		y1 = tmp;
	}
	for(int tileX = x1; tileX <= x2; tileX++) {
		for(int tileY = y1; tileY <= y2; tileY++) {
			map->setProperties(tileX, tileY, true, true);
		}
	}
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2) {
	dig(x1, y1, x2, y2);
	if(first) {
		// put the player in the first room
		engine.player->x = (x1 + x2) / 2;
		engine.player->y = (y1 + y2) / 2;
	}
	else {
		TCODRandom *rng = TCODRandom::getInstance();
		int nbMonsters = rng->getInt(0, MAX_ROOM_MONSTERS);
		while (nbMonsters > 0) {
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);
			if(canWalk(x, y)) {
				addMonster(x, y);
			}
			nbMonsters--;
		}

		// add items
		int nbItems = rng->getInt(0, MAX_ROOM_ITEMS);
		while(nbItems > 0) {
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);
			if(canWalk(x, y)) {
				addItem(x, y);
			}
			nbItems--;
		}
	}
}

void Map::render() const {
	static const TCODColor darkWall(0, 0, 100);
	static const TCODColor darkGround(50, 50, 150);
	static const TCODColor lightWall(130, 110, 50);
	static const TCODColor lightGround(200, 180, 50);

	for(int x = 0; x < width; x++)	{
		for(int y = 0; y < height; y++) {
			if(isInFov(x, y)) {
				TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? lightWall : lightGround);
			}
			else if(isExplored(x, y)) {
				TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? darkWall : darkGround);
			}
		}
	}
}