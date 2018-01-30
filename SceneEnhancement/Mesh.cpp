#include "Mesh.h"
#include <QOpenGLFunctions>

Mesh::Mesh()
	:VBO(QOpenGLBuffer::VertexBuffer)
	, EBO(QOpenGLBuffer::IndexBuffer)
{
}

Mesh::Mesh(QVector<Vertex> vertices, QVector<GLuint> indices, Material *material/*QVector<Texture*> textures*/)
	: VBO(QOpenGLBuffer::VertexBuffer)
	, EBO(QOpenGLBuffer::IndexBuffer)
{
	//initializeOpenGLFunctions();
	this->Vertices = vertices;
	this->Indices = indices;
	//this->Textures = textures;
	this->MeshMaterial = material;
	this->updateNormals();
	this->setupRender();
}


void Mesh::Draw(QOpenGLShaderProgram *program)
{		
	setupShaderProgram(program);
	VAO.bind();	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glDrawElements(GL_TRIANGLES, this->Indices.size() , GL_UNSIGNED_INT, 0);	
	glDrawElements(GL_TRIANGLES, this->Indices.size(), GL_UNSIGNED_INT, 0);
	VAO.release();
}

void Mesh::setupRender()
{	
	if(!VAO.isCreated())	
		VAO.create();
	if (!VBO.isCreated())
		VBO.create();
	if (!EBO.isCreated())
		EBO.create();

	VAO.bind();
	VBO.bind();
	VBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	VBO.allocate(Vertices.constData(), Vertices.size()*sizeof(Vertex));

	EBO.bind();
	EBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	EBO.allocate(Indices.constData(), Indices.size()*sizeof(GLuint));
	
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	f->glEnableVertexAttribArray(0); // location = 0
	f->glVertexAttribPointer(0, Vertex::PositionTupleSize, GL_FLOAT, GL_FALSE, Vertex::stride(), (GLvoid*)Vertex::positionOffset());
	f->glEnableVertexAttribArray(1); // location = 1
	f->glVertexAttribPointer(1, Vertex::NormalTupleSize, GL_FLOAT, GL_FALSE, Vertex::stride(), (GLvoid*)Vertex::normalOffset());
	f->glEnableVertexAttribArray(2); // location = 2
	f->glVertexAttribPointer(2, Vertex::TextureTupleSize, GL_FLOAT, GL_FALSE, Vertex::stride(), (GLvoid*)Vertex::textureOffset());

	VBO.release();
	VAO.release();
}

void Mesh::GetMinMaxCoordinates(QVector3D& min, QVector3D& max)
{
	if (this->Vertices.size() == 0)
	{
		qDebug("empty mesh");
	}
	min = this->Vertices[0].position();
	max = this->Vertices[0].position();
	for (size_t i = 0; i < this->Vertices.size(); i++)
	{
		float x = this->Vertices[i].position().x();
		float y = this->Vertices[i].position().y();
		float z = this->Vertices[i].position().z();
		min.setX(x < min.x() ? x : min.x());
		min.setY(y < min.y() ? y : min.y());
		min.setZ(z < min.z() ? z : min.z());
		max.setX(x > max.x() ? x : max.x());
		max.setY(y > max.y() ? y : max.y());
		max.setZ(z > max.z() ? z : max.z());		
	}
}

float Mesh::GetArea()
{
	float area = 0;
	int faceNum = this->Indices.size() / 3;	
	for (size_t i = 0; i < faceNum; i++)
	{
		QVector3D v1 = this->Vertices[this->Indices[i * 3 + 0]].position();
		QVector3D v2 = this->Vertices[this->Indices[i * 3 + 1]].position();
		QVector3D v3 = this->Vertices[this->Indices[i * 3 + 2]].position();

		float a = v1.distanceToPoint(v2);
		float b = v2.distanceToPoint(v3);
		float c = v3.distanceToPoint(v1);
		float p = (a + b + c) / 2;
		area += sqrt(abs(p*(p - a)*(p - b)*(p - c)));
	}

	return area;
}

void Mesh::updateNormals()
{
	// reset normals
	for (size_t i = 0; i < this->Vertices.size(); i++)
	{
		this->Vertices[i].setNormal(QVector3D());
	}

	int faceNum = this->Indices.size() / 3;
	QVector<QVector3D> faceNormals;
	for (size_t i = 0; i < faceNum; i++)
	{
		QVector3D v1 = this->Vertices[this->Indices[i * 3 + 0]].position();
		QVector3D v2 = this->Vertices[this->Indices[i * 3 + 1]].position();
		QVector3D v3 = this->Vertices[this->Indices[i * 3 + 2]].position();

		QVector3D edge12 = v1 - v2;
		QVector3D edge23 = v2 - v3;

		QVector3D norm = QVector3D::crossProduct(edge12, edge23).normalized();		
		faceNormals.push_back(norm);
	}

	for (size_t i = 0; i < faceNormals.size(); i++)
	{
		this->Vertices[this->Indices[i * 3 + 0]].setNormal(this->Vertices[this->Indices[i * 3 + 0]].normal() + faceNormals[i]);
		this->Vertices[this->Indices[i * 3 + 1]].setNormal(this->Vertices[this->Indices[i * 3 + 1]].normal() + faceNormals[i]);
		this->Vertices[this->Indices[i * 3 + 2]].setNormal(this->Vertices[this->Indices[i * 3 + 2]].normal() + faceNormals[i]);
	}	
	
}

void Mesh::setupShaderProgram(QOpenGLShaderProgram* program)
{
	program->setUniformValue("material.useAmbientMap", MeshMaterial->Ambient->UseMap);
	program->setUniformValue("material.useDiffuseMap", MeshMaterial->Diffuse->UseMap);
	program->setUniformValue("material.useSpecularMap", MeshMaterial->Specular->UseMap);
	program->setUniformValue("material.ambientColor", MeshMaterial->Ambient->Color);
	program->setUniformValue("material.diffuseColor", MeshMaterial->Diffuse->Color);
	program->setUniformValue("material.specularColor", MeshMaterial->Specular->Color);
	program->setUniformValue("material.shininess", MeshMaterial->Shininess);
	program->setUniformValue("material.opacity", MeshMaterial->Opacity);

	for (size_t i = 0; i < MeshMaterial->Ambient->Textures.size(); i++)
	{
		MeshMaterial->Ambient->Textures[i]->Bind();
		program->setUniformValue("material.ambient", MeshMaterial->Ambient->Textures[i]->id);
	}
	for (size_t i = 0; i < MeshMaterial->Diffuse->Textures.size(); i++)
	{
		MeshMaterial->Diffuse->Textures[i]->Bind();
		program->setUniformValue("material.diffuse", MeshMaterial->Diffuse->Textures[i]->id);
	}
	for (size_t i = 0; i < MeshMaterial->Specular->Textures.size(); i++)
	{
		MeshMaterial->Specular->Textures[i]->Bind();
		program->setUniformValue("material.specular", MeshMaterial->Specular->Textures[i]->id);
	}
}
