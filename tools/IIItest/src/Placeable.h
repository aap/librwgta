class CPlaceable
{
	// disable allocation
	static void *operator new(size_t) { assert(0); }
	static void operator delete(void*, size_t) { assert(0); }
public:
	CMatrix m_matrix;

	CPlaceable(void);
	virtual ~CPlaceable(void);
	CVector *GetPosition(void) { return m_matrix.GetPosition(); }
};
