#ifndef MODEL_H
#define MODEL_H
#include <QtCore/qstring.h>
#include <QtGui/QOpenGLShader>
#include "Mesh.h"

#include <assimp/scene.h>
#include "Material.h"
#include "BoundingBox.h"

class Model
{
public:
	/* constructors and destructors*/
	Model();
	Model(QString path);
	Model(QString path, QVector3D translate, QVector3D rotate, float scale);
	virtual ~Model();

	/* Render */
	virtual void Draw(QOpenGLShaderProgram *program);
	
	/* Attributes */
	void SetTranslation(QVector3D translate);
	void SetScale(float scale);
	void SetRotation(QVector3D rotate);
	QVector3D& GetTranslate() { return m_translate; };
	QVector3D& GetRotate() { return m_rotate; };
	float GetScale() const	{ return m_scale; };
	BoundingBox *boundingBox;
	void updateMeshNormals();
	/* assimp data structure*/
	const aiScene *AiScene;

	/* Export model using assimp*/
	void ExportModel(QString name);
	void UpdateBoundingBoxWorldCoordinates();
protected:
	void init();

	QVector3D m_translate;
	QVector3D m_rotate;
	float m_scale;
	QMatrix4x4 modelMatrix;
		
	void updateBoundingBox();
	void GetMinMaxCoordinates(QVector3D &min, QVector3D &max);
	
	
	// 记录原始的位移信息
	QVector3D org_min;
	QVector3D org_max;

	QVector<Mesh*> meshes;
	QVector<Texture*> textures_loaded;
	QMap<QString, Material*> material_assets;	
	QString directory;

	/* Load model using assimp */
	void loadModel(QString path);	
	void processNode(aiNode* node, const aiScene *scene);
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene);
	QVector<Texture*> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType typeName);
	QOpenGLTexture* TextureFromFile(QString path, QString directory);
		
	void updateVertexPosition();
	
};


#endif
