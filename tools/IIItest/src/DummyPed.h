class CDummyPed : public CDummy
{
public:
	void *operator new(size_t);
	void operator delete(void*, size_t);

	CDummyPed(void);

	// from CPlaceable
	~CDummyPed(void);
};
