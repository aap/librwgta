class CPhysical : public CEntity
{
	// disable allocation
	static void *operator new(size_t) { assert(0); return nil; }
	static void operator delete(void*, size_t) { assert(0); }

	CPtrNode *m_movingListNode;
public:
	CPhysical(void);
	void AddToMovingList(void);

	// from CPlaceable
	~CPhysical(void);
};
