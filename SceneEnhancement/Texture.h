#ifndef TEXTURE_H
#define TEXTURE_H
#include <QOpenGLTexture>
#include <QVector3D>
#include <assimp/types.h>

enum TextureType
{
	AmbientTexture,
	DiffuseTexture,
	SpecularTexture
};
class Texture
{
public:
	Texture();
	QVector3D color;
	QOpenGLTexture *texture;
	GLint id;
	TextureType type;
	aiString path;
	QString fullpath;
	float Transparency;
	void Bind() { texture->bind(id); };


};

#endif

