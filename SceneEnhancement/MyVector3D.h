#ifndef MYVECTOR_H
#define MYVECTOR_H

#include<cmath>

using namespace std;

namespace DataStruct
{
	class MyVector3D
	{
	public:
		MyVector3D();
		MyVector3D(float x, float y, float z);
		MyVector3D(float* data);
		MyVector3D(const MyVector3D& o);

		float dot(const MyVector3D o) const;
		MyVector3D cross(const MyVector3D o) const;
		float angle(const MyVector3D o);
		MyVector3D normalized();
		void normalize();
		float length();

		float& operator[] (const int index);
		const float& operator[] (const int index) const;
		void operator=(const MyVector3D o);
		MyVector3D operator+(const MyVector3D o) const;
		MyVector3D operator-(const MyVector3D o) const;
		MyVector3D operator*(const double o) const;
		MyVector3D operator/(const double o) const;

	private:
		float data[3];
	};

}
#endif