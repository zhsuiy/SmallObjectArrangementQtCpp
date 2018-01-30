#ifndef SCENE_H
#define SCENE_H

#include <QVector>
#include "FurnitureObject.h"

class Scene
{
public:
	Scene();
	~Scene();

private:
	QVector<FurnitureObject> m_furnitures;

};

#endif
