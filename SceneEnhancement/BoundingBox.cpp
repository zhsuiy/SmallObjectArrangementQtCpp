#include "BoundingBox.h"

BoundingBox::BoundingBox(QVector3D leftBottomBack, QVector3D rightUpFront) :Mesh()
{

	this->m_left_bottom_back = leftBottomBack;
	this->m_right_up_front = rightUpFront;
	m_width = abs(rightUpFront.x() - leftBottomBack.x());
	m_height = abs(rightUpFront.y() - leftBottomBack.y());
	m_depth = abs(rightUpFront.z() - leftBottomBack.z());
	setupData();
	setupRender();

}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::Draw(QOpenGLShaderProgram* program)
{
	setupShaderProgram(program);
	VAO.bind();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(5);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glDrawElements(GL_TRIANGLES, this->Indices.size() , GL_UNSIGNED_INT, 0);	
	glDrawElements(GL_LINES, this->Indices.size(), GL_UNSIGNED_INT, 0);
	VAO.release();
}

void BoundingBox::UpdateWorldCoordinates(QMatrix4x4& modelMatrix)
{
	QVector3D tmplbb = modelMatrix * m_left_bottom_back;
	QVector3D tmpruf = modelMatrix * m_right_up_front;
	QVector3D tmplub(m_left_bottom_back.x(), m_left_bottom_back.y(), m_right_up_front.z());
	QVector3D tmprub(m_right_up_front.x(), m_left_bottom_back.y(), m_left_bottom_back.z());
	tmplub = modelMatrix * tmplub;
	tmprub = modelMatrix * tmprub;

	float min_x, min_y, min_z, max_x, max_y, max_z;
	min_x = qMin(qMin(tmplbb.x(), tmpruf.x()), qMin(tmplub.x(), tmprub.x()));
	min_y = qMin(tmplbb.y(), tmpruf.y());
	min_z = qMin(qMin(tmplbb.z(), tmpruf.z()), qMin(tmplub.z(), tmprub.z()));
	max_x = qMax(qMax(tmplbb.x(), tmpruf.x()),qMax(tmplub.x(), tmprub.x()));
	max_y = qMax(tmplbb.y(), tmpruf.y());
	max_z = qMax(qMax(tmplbb.z(), tmpruf.z()),qMax(tmplub.z(), tmprub.z()));
	m_world_left_bottom_back = QVector3D(min_x, min_y, min_z);
	m_world_right_up_front = QVector3D(max_x, max_y, max_z);
	
	m_world_width = max_x - min_x;
	m_world_depth = max_z - min_z;
	m_world_height = max_y - min_y;
}

void BoundingBox::setupData()
{
	updateVertices();
	updateTextures();
	// indices will not change
	if (Indices.isEmpty())
	{
		GLuint arr_indices[] = {
			0,1,
			1,2,
			2,3,
			3,0,
			4,5,
			5,6,
			6,7,
			7,4,
			0,4,
			3,7,
			1,5,
			2,6
		};
		for (size_t i = 0; i < 24; i++)
		{
			Indices.push_back(arr_indices[i]);
		}
	}
}

void BoundingBox::updateVertices()
{
	float neg_x = m_left_bottom_back.x();
	float neg_y = m_left_bottom_back.y();
	float neg_z = m_left_bottom_back.z();
	float pos_x = m_right_up_front.x();
	float pos_y = m_right_up_front.y();
	float pos_z = m_right_up_front.z();
	Vertices.clear();

	// x
	Vertices.push_back(Vertex(QVector3D(neg_x, neg_y, pos_z), QVector3D(0, 0, 1))); // front
	Vertices.push_back(Vertex(QVector3D(pos_x, neg_y, pos_z), QVector3D(0, 0, 1)));
	Vertices.push_back(Vertex(QVector3D(pos_x, pos_y, pos_z), QVector3D(0, 0, 1)));
	Vertices.push_back(Vertex(QVector3D(neg_x, pos_y, pos_z), QVector3D(0, 0, 1)));

	Vertices.push_back(Vertex(QVector3D(neg_x, neg_y, neg_z), QVector3D(0, 0, 1))); // back
	Vertices.push_back(Vertex(QVector3D(pos_x, neg_y, neg_z), QVector3D(0, 0, 1)));
	Vertices.push_back(Vertex(QVector3D(pos_x, pos_y, neg_z), QVector3D(0, 0, 1)));
	Vertices.push_back(Vertex(QVector3D(neg_x, pos_y, neg_z), QVector3D(0, 0, 1)));
}

void BoundingBox::updateTextures()
{
	MeshMaterial = new Material();
	MeshMaterial->Diffuse = new MaterialElement(QVector3D(0.5,0.5,0.8));
}
