#ifndef MESH_H
#define MESH_H

#include <QtCore/qvector.h>
#include <QtWidgets/QAction>
#include "Vertex.h"
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include "Texture.h"
#include "Material.h"

class Shader;

class Mesh 
{
public:
	/* Constructors */
	Mesh();	
	Mesh(QVector<Vertex> vertices, QVector<GLuint> indices, Material *material);
	
	/* Mesh Data*/
	QVector<Vertex> Vertices;	// ��Ŷ�����Ϣ
	QVector<GLuint> Indices;	// ��Ŷ����index	
	Material *MeshMaterial;

	/* Functions */
	virtual void Draw(QOpenGLShaderProgram *program);	// ��mesh������
	void setupRender();	// ��ʼ������buffer,���ݴ�Assimp��
	
	void GetMinMaxCoordinates(QVector3D &min, QVector3D &max);
	float GetArea();
protected:
	/* Render data */
	QOpenGLBuffer VBO;	// vertex buffer object, ����ʵ�ʵ�����
	QOpenGLBuffer EBO;	// element buffer object, ����index
	QOpenGLVertexArrayObject VAO;	// vertex attribute object, ����Ķ�������	
	
	void updateNormals(); // ���¶��㷨����
	void setupShaderProgram(QOpenGLShaderProgram *program);

};


#endif

