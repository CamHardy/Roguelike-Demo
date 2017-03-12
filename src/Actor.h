class Actor {
public:
	int x, y;					// position
	int ch;						// ascii code
	TCODColor col;				// color
	const char *name;			// the actor's name
	bool blocks;				// does actor block movement
	Attacker *attacker;			// can deal damage
	Destructible *destructible;	// can be damaged
	Ai *ai;						// can self-update
	Pickable *pickable;			// can be picked up
	Container *container;		// can contain other actors

	Actor(int x, int y, int ch, const char *name, const TCODColor &col);
	~Actor();
	void update();
	void render() const;
	float getDistance(int cx, int cy) const;
};