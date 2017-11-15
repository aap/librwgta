class CDummyObject : public CDummy
{
public:
	void *operator new(size_t);
	void operator delete(void*, size_t);

	CDummyObject(void);

	// from CPlaceable
	~CDummyObject(void);
};
