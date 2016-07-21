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
};
