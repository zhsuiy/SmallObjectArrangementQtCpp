#pragma once

#include "Mesh.h"

class BoundingBox:public Mesh
{
public:
	BoundingBox(QVector3D leftBottomBack, QVector3D rightUpFront);
	~BoundingBox();
	float Width() const	{	return m_width;	};
	float Height() const { return m_height; };
	float Depth() const { return m_depth; };
	QVector3D& LeftBottomBack() { return m_left_bottom_back; };
	QVector3D& RightUpFront() { return m_right_up_front; };
	QVector3D& WorldLeftBottomBack() { return m_world_left_bottom_back; }
	QVector3D& WorldRightUpFront() { return m_world_right_up_front; }
	//QVector3D& WorldCenter() { return m_world_center; }
	float WorldWidth() const { return m_world_width; };
	float WorldDepth() const { return m_world_depth; };
	float WorldHeight() const { return m_world_height; };
	virtual void Draw(QOpenGLShaderProgram *program);
	void UpdateWorldCoordinates(QMatrix4x4 &modelMatrix);

private:
	float m_width;
	float m_height;
	float m_depth;
	QVector3D m_left_bottom_back;
	QVector3D m_right_up_front;
	

	QVector3D m_world_left_bottom_back;
	QVector3D m_world_right_up_front;
	//QVector3D m_world_center;
	float m_world_width;
	float m_world_depth;
	float m_world_height;

	void setupData();
	void updateVertices();
	void updateTextures();
};



