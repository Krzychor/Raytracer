#ifndef Vec3Guard
#define Vec3Guard

//vector implementation
template<typename T>
class Vec3
{
public:

	T x, y, z;
	Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
	Vec3(T x1, T y1, T z1) : x(x1), y(y1), z(z1) {}
	Vec3(T val) : x(val), y(val), z(val) {}
	
	void normalize()
	{
		float l = length2();
		if(l > 0) {
			float inv = 1.0f/sqrt(l);
			x *= inv; 
			y *= inv;
			z *= inv;
		}
	}
	
	float length2() const { //squared lenght
		return x*x+y*y+z*z;
	}
	
	float length() const {
		return sqrt(length2());
	}
	
	Vec3<T> operator*(const T& Val) const {
		return Vec3<T>(x*Val, y*Val, z*Val);
	}
	Vec3<T> operator*(const Vec3<T>& vec) const {
		return Vec3<T>(x*vec.x, y*vec.y, z*vec.z);
	}
	Vec3<T> operator+(const Vec3<T>& vec) const {
		return Vec3<T>(x+vec.x, y+vec.y, z+vec.z);
	}
	Vec3<T> operator+(const T& Val) const {
		return Vec3<T>(x+Val, y+Val, z+Val);
	} 
    Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; } 
    Vec3<T>& operator *= (const Vec3<T> &v) { x *= v.x, y *= v.y, z *= v.z; return *this; } 
    Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); } 
    Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; } 
};

	template<typename T>
	Vec3<T> operator*(const T& val, const Vec3<T> vec) {
		return Vec3<T>(vec.x*val, vec.y*val, vec.z*val);
	}


typedef Vec3<float> Vec3f; 

#endif