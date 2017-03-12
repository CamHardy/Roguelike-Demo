class Container {
public:
	int size;	// maximum number of stored actors, 0 = unlimited
	TCODList<Actor *> inventory;

	Container(int size);
	~Container();
	bool add(Actor *actor);
	void remove(Actor *actor);
};