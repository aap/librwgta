class CVector
{
public:
	float x, y, z;
	CVector(void) {}
	CVector(float x, float y, float z) : x(x), y(y), z(z) {}
	CVector(rw::V3d const &v) : x(v.x), y(v.y), z(v.z) {}
	float Magnitude(void) { return sqrt(x*x + y*y + z*z); }
	float MagnitudeSqr(void) { return x*x + y*y + z*z; }
	float Magnitude2D(void) { return sqrt(x*x + y*y); }
	void Normalise(void){
		float sq = MagnitudeSqr();
		if(sq > 0.0f){
			float invsqrt = 1.0f/sqrt(sq);
			x *= invsqrt;
			y *= invsqrt;
			z *= invsqrt;
		}else
			x = 0.0f;
	}
	rw::V3d ToRW(void){
		return rw::V3d(x, y, z);
	}
	void operator=(rw::V3d const &rhs){
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
	}
	CVector operator-(const CVector &rhs) const {
		return CVector(x-rhs.x, y-rhs.y, z-rhs.z);
	}
	CVector operator+(const CVector &rhs) const {
		return CVector(x+rhs.x, y+rhs.y, z+rhs.z);
	}
};
