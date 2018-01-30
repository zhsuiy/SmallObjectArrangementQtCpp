#include "MyVector3D.h"

DataStruct::MyVector3D::MyVector3D()
{
	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
}

DataStruct::MyVector3D::MyVector3D(float x, float y, float z)
{
	data[0] = x;
	data[1] = y;
	data[2] = z;
}

DataStruct::MyVector3D::MyVector3D(float* data)
{
	this->data[0] = data[0];
	this->data[1] = data[1];
	this->data[2] = data[2];
}

DataStruct::MyVector3D::MyVector3D(const MyVector3D& o)
{
	data[0] = o[0];
	data[1] = o[1];
	data[2] = o[2];
}

float DataStruct::MyVector3D::dot(const MyVector3D o) const
{
	return data[0]*o[0]+data[1]*o[1]+data[2]*o[2];
}

DataStruct::MyVector3D DataStruct::MyVector3D::cross(const MyVector3D o) const
{
	MyVector3D result;
	result[0] = data[1] * o[2] - data[2] * o[1];
	result[1] = data[2] * o[0] - data[0] * o[2];
	result[2] = data[0] * o[1] - data[1] * o[0];
	return result;
}

float DataStruct::MyVector3D::angle(const MyVector3D o)
{
	MyVector3D a,b(o);
	a = normalized();
	b.normalize();
	
	float x = a.dot(b);
	if(x>1) x=1.0;
	return acos(x);
}

DataStruct::MyVector3D DataStruct::MyVector3D::normalized()
{
	MyVector3D o(data);
	o.normalize();
	return o;
}

void DataStruct::MyVector3D::normalize()
{

	float l = length();
	if (abs(l) < 1e-8) {
		if ((data[0] >= data[1]) && (data[0] >= data[2])) {
			data[0] = 1.0f;
			data[1] = data[2] = 0.0f;
		} else
			if (data[1] >= data[2]) {
				data[1] = 1.0f;
				data[0] = data[2] = 0.0f;
			} else {
				data[2] = 1.0f;
				data[0] = data[1] = 0.0f;
			}
	} else {
		float m = 1.0f / l;
		data[0] *= m;
		data[1] *= m;
		data[2] *= m;
	}
}

float DataStruct::MyVector3D::length()
{
	return sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]);
}

float& DataStruct::MyVector3D::operator[] (int index)
{
	return data[index];
}

const float& DataStruct::MyVector3D::operator[] (const int index) const
{
	return data[index];
}

DataStruct::MyVector3D DataStruct::MyVector3D::operator+(const MyVector3D o) const 
{
	MyVector3D result;
	result[0] = data[0]+o[0];
	result[1] = data[1]+o[1];
	result[2] = data[2]+o[2];

	return result;
}

DataStruct::MyVector3D DataStruct::MyVector3D::operator-(const MyVector3D o) const
{
	MyVector3D result;
	result[0] = data[0]-o[0];
	result[1] = data[1]-o[1];
	result[2] = data[2]-o[2];

	return result;
}

DataStruct::MyVector3D DataStruct::MyVector3D::operator*(const double o) const
{
	MyVector3D result;
	result[0] = data[0]*o;
	result[1] = data[1]*o;
	result[2] = data[2]*o;

	return result;
}

DataStruct::MyVector3D DataStruct::MyVector3D::operator/(const double o) const
{
	MyVector3D result;
	result[0] = data[0]/o;
	result[1] = data[1]/o;
	result[2] = data[2]/o;

	return result;
}

void DataStruct::MyVector3D::operator=(const MyVector3D o)
{
	data[0] = o[0];
	data[1] = o[1];
	data[2] = o[2];
}