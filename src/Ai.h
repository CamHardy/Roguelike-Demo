class Ai {
public:
	virtual ~Ai() {};
	virtual void update(Actor *owner) = 0;
};

class PlayerAi : public Ai {
public:
	void update(Actor *owner);

protected:
	bool moveOrAttack(Actor *owner, int targetX, int targetY);
	void handleActionKey(Actor *owner, int ascii);
	Actor *chooseFromInventory(Actor *owner);
};

class MonsterAi : public Ai {
public:
	MonsterAi();
	void update(Actor *owner);

protected:
	int moveCount;
	void moveOrAttack(Actor *owner, int targetX, int targetY);
};

class ConfusedMonsterAi : public Ai {
public:
	ConfusedMonsterAi(int nbTurns, Ai *oldAi);
	void update(Actor *owner);
protected:
	int nbTurns;
	Ai *oldAi;
};