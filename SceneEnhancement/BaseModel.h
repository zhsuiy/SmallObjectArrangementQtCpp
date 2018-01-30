#ifndef BASE_OBJECT_H
#define BASE_OBJECT_H
#include <QtCore/qstring.h>
#include "Vertex.h"

#include "Model.h"


class BaseModel:public Model
{
public:
	BaseModel();
	~BaseModel();
	int ID;
	QString Path;
	

};

#endif
