class CCollision
{
public:
	static eLevelName ms_collisionInMemory;

	static void Update(void);
	static void LoadCollisionWhenINeedIt(bool changeLevel);
};
