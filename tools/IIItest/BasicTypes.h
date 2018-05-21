#ifndef _BASICTYPES_H_
#define _BASICTYPES_H_

class CVector
{
public:
	float x, y, z;
	CVector(void) {}
	CVector(float x, float y, float z) : x(x), y(y), z(z) {}
	CVector(rw::V3d const &v) : x(v.x), y(v.y), z(v.z) {}
	float Magnitude(void) const { return sqrt(x*x + y*y + z*z); }
	float MagnitudeSqr(void) const { return x*x + y*y + z*z; }
	float Magnitude2D(void) const { return sqrt(x*x + y*y); }
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
		return rw::makeV3d(x, y, z);
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
	CVector operator*(float t) const {
		return CVector(x*t, y*t, z*t);
	}
};

class CVector2D
{
public:
	float x, y;
	CVector2D(void) {}
	CVector2D(float x, float y) : x(x), y(y) {}
	CVector2D(const CVector &v) : x(v.x), y(v.y) {}
	float Magnitude(void) const { return sqrt(x*x + y*y); }
	float MagnitudeSqr(void) const { return x*x + y*y; }

	void Normalise(void){
		float sq = MagnitudeSqr();
		if(sq > 0.0f){
			float invsqrt = 1.0f/sqrt(sq);
			x *= invsqrt;
			y *= invsqrt;
		}else
			x = 0.0f;
	}
	CVector2D operator-(const CVector2D &rhs) const {
		return CVector2D(x-rhs.x, y-rhs.y);
	}
	CVector2D operator+(const CVector2D &rhs) const {
		return CVector2D(x+rhs.x, y+rhs.y);
	}
	CVector2D operator*(float t) const {
		return CVector2D(x*t, y*t);
	}
};

inline float DotProduct(const CVector &v1, const CVector &v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}
inline CVector CrossProduct(const CVector &v1, const CVector &v2) {
	return CVector(v1.y*v2.z - v1.z*v2.y,
		v1.z*v2.x - v1.x*v2.z,
		v1.x*v2.y - v1.y*v2.x);
}
inline float CrossProduct2D(const CVector2D &v1, const CVector2D &v2) {
	return v1.x*v2.y - v1.y*v2.x;
}

class CMatrix
{
public:
	rw::Matrix m_matrix;
	rw::Matrix *m_attachment;
	bool m_hasRwMatrix;	// are we the owner?

	CMatrix(void){
		m_attachment = nil;
		m_hasRwMatrix = false;
	}
	CMatrix(CMatrix const &m){
		m_attachment = nil;
		m_hasRwMatrix = false;
		*this = m;
	}
	CMatrix(rw::Matrix *matrix, bool attach){
		m_attachment = nil;
		Attach(matrix, attach);
	}
	~CMatrix(void){
		if(m_hasRwMatrix && m_attachment)
			m_attachment->destroy();
	}
	void Attach(rw::Matrix *matrix, bool attach){
		if(m_hasRwMatrix && m_attachment)
			m_attachment->destroy();
		m_attachment = matrix;
		m_hasRwMatrix = attach;
		Update();
	}
	void AttachRW(rw::Matrix *matrix, bool attach){
		if(m_hasRwMatrix && m_attachment)
			m_attachment->destroy();
		m_attachment = matrix;
		m_hasRwMatrix = attach;
		UpdateRW();
	}
	void Detach(void){
		if(m_hasRwMatrix && m_attachment)
			m_attachment->destroy();
		m_attachment = nil;
	}
	void Update(void){
		m_matrix = *m_attachment;
	}
	void UpdateRW(void){
		if(m_attachment)
			*m_attachment = m_matrix;
	}
	void operator=(CMatrix const &rhs){
		m_matrix = rhs.m_matrix;
		if(m_attachment)
			UpdateRW();
	}

	CVector *GetPosition(void){ return (CVector*)&m_matrix.pos; }
	void SetScale(float s){
		m_matrix.right.x = s;
		m_matrix.right.y = 0.0f;
		m_matrix.right.z = 0.0f;

		m_matrix.up.x = 0.0f;
		m_matrix.up.y = s;
		m_matrix.up.z = 0.0f;

		m_matrix.at.x = 0.0f;
		m_matrix.at.y = 0.0f;
		m_matrix.at.z = s;

		m_matrix.pos.x = 0.0f;
		m_matrix.pos.y = 0.0f;
		m_matrix.pos.z = 0.0f;
	}
};

inline CVector
operator*(const CMatrix &mat, const CVector &vec)
{
	return CVector(
		mat.m_matrix.right.x * vec.x + mat.m_matrix.up.x * vec.y + mat.m_matrix.at.x * vec.z + mat.m_matrix.pos.x,
		mat.m_matrix.right.y * vec.y + mat.m_matrix.up.y * vec.y + mat.m_matrix.at.y * vec.z + mat.m_matrix.pos.y,
		mat.m_matrix.right.z * vec.z + mat.m_matrix.up.z * vec.y + mat.m_matrix.at.z * vec.z + mat.m_matrix.pos.z);
}

class CRGBA
{
public:
	uint8 r, g, b, a;
	CRGBA(void) { }
	CRGBA(uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a) { }
};

#endif
