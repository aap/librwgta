template<typename T, int n>
class CStore
{
public:
	int allocPtr;
	T store[n];

	T *alloc(void){
		if(this->allocPtr >= n)
			printf("Size of this thing:%d needs increasing\n", n);
		return &this->store[this->allocPtr++];
	}
	void clear(void){
		this->allocPtr = 0;
	}
};

template<typename T, typename U>
class CPool
{
	U     *m_entries;
	union Flags {
		struct {
			uchar id   : 7;
			uchar free : 1;
		};
			uchar u;
	}     *m_flags;
	int    m_size;
	int    m_allocPtr;

public:
	CPool(int size){
		m_entries = (U*)malloc(sizeof(U)*size);
		m_flags = (Flags*)malloc(sizeof(Flags)*size);
		m_size = size;
		m_allocPtr = 0;
		for(int i = 0; i < size; i++){
			m_flags[i].id   = 0;
			m_flags[i].free = 1;
		}
	}
	int GetSize(void) { return m_size; }
	T *New(void){
		bool wrapped = false;
		do
			if(++m_allocPtr == m_size){
				if(wrapped)
					return nil;
				wrapped = true;
				m_allocPtr = 0;
			}
		while(!m_flags[m_allocPtr].free);
		m_flags[m_allocPtr].free = 0;
		m_flags[m_allocPtr].id++;
		return (T*)&m_entries[m_allocPtr];
	}
	T *New(int handle){
		T *entry = (T*)m_entries[handle>>8];
		SetNotFreeAt(handle);
		return entry;
	}
	void SetNotFreeAt(int handle){
		int idx = handle>>8;
		m_flags[idx].free = 0;
		m_flags[idx].id = handle & 0x7F;
		for(m_allocPtr = 0; m_allocPtr < m_size; m_allocPtr++)
			if(m_flags[m_allocPtr].free)
				return;
	}
	void Delete(T *entry){
		int i = GetJustIndex(entry);
		m_flags[i].free = 1;
		if(i < m_allocPtr)
			m_allocPtr = i;
	}
	T *GetSlot(int i){
		return m_flags[i].free ? nil : (T*)&m_entries[i];
	}
	T *GetAt(int handle){
		return m_flags[handle>>8].u == handle & 0xFF ?
		       (T*)&m_entries[handle>>8] : nil;
	}
	int GetIndex(T *entry){
		int i = GetJustIndex(entry);
		return m_flags[i].u + (i<<8);
	}
	int GetJustIndex(T *entry){
		return (U*)entry - m_entries;
	}
	int GetNoOfUsedSpaces(void){
		int i;
		int n = 0;
		for(i = 0; i < m_size; i++)
			if(!m_flags[i].free)
				n++;
		return n;
	}
};
