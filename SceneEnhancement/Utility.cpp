#include "Utility.h"
#include <iostream>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qfile.h>
#include "Global.h"
#include "Parameter.h"
#include <QtCore/qdir.h>
#include <ctime>
#include <iterator>
#include <vector>
#include "ProbLearning.h"
#include <algorithm>

QVector3D Utility::Str2Vec3D(QString &str)
{
	float x, y, z;
	QStringList numbers = str.split(' ', QString::SkipEmptyParts);
	if (numbers.size() == 1)
		x = y = z = numbers[0].toFloat();
	else if (numbers.size() == 3)
	{
		x = numbers[0].toFloat();
		y = numbers[1].toFloat();
		z = numbers[2].toFloat();
	}
	else
	{
		std::cout << "Invalid direction parameter\n";
	}	
	return QVector3D(x,y,z);
}

QVector<FurnitureModel*> Utility::ParseFurnitureModels(QString &path)
{
	Parameter *para = Parameter::GetParameterInstance();
	Assets *assets = Assets::GetAssetsInstance();
	QVector<FurnitureModel*> furniture_models;
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split('=', QString::SkipEmptyParts);
		if (parts.size() == 0) // skip blank line
			continue;
		// room area
		if (parts[0].compare("RoomWidth", Qt::CaseInsensitive) == 0)
			assets->RoomWidth = QStr2Float(parts[1]);
		if (parts[0].compare("RoomHeight", Qt::CaseInsensitive) == 0)
			assets->RoomHeight = QStr2Float(parts[1]);
		if (parts[0].compare("RoomDepth", Qt::CaseInsensitive) == 0)
			assets->RoomDepth = QStr2Float(parts[1]);
		if (QStrCmp(parts[0], "WallColor"))
			assets->WallColor = Str2Vec3D(parts[1]);
		if (QStrCmp(parts[0], "FloorTexture"))
			assets->FloorTexture = parts[1].trimmed();
		if (parts[0].compare("Furniture",Qt::CaseInsensitive) == 0)
		{
			FurnitureType type = parts[1].trimmed();
			if (!para->FurnitureTypes.contains(type))
			{
				qWarning("%s is not included",type.toStdString().c_str());
				continue;
			}
			FurnitureName name;
			float scale;
			QVector3D translate, rotate;
			QVector<FurnitureLocationType> locationTypes;
			for (size_t i = 0; i < 5; i++)
			{
				QByteArray inner_line = file->readLine();
				QString inner_str(inner_line);
				QStringList inner_parts = inner_str.split('=', QString::SkipEmptyParts);
				if (inner_parts.size() == 0)
					continue;
				if (QStrCmp(inner_parts[0],"Path"))
					name = inner_parts[1].trimmed();
				if (QStrCmp(inner_parts[0],"Scale"))
					scale = QStr2Float(inner_parts[1]);
				if (QStrCmp(inner_parts[0],"Translate"))
					translate = Str2Vec3D(inner_parts[1]);
				if (QStrCmp(inner_parts[0], "Rotate"))
					rotate = Str2Vec3D(inner_parts[1]);
				if (QStrCmp(inner_parts[0], "Location"))
					locationTypes = ParseLocationTypes(inner_parts[1]);
					
			}
			FurnitureModel* model = new FurnitureModel(type, name, translate, rotate,locationTypes, scale);
			furniture_models.push_back(model);			
		}
	}
	file->close();
	delete file;
	return furniture_models;
}

QVector<DecorationModel*> Utility::ParseDecorationModels(QString& path)
{
	Parameter *para = Parameter::GetParameterInstance();
	QVector<DecorationModel*> decoration_models;
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split('=', QString::SkipEmptyParts);
		if (parts.size() == 0) // skip blank line
			continue;
		
		if (parts[0].compare("Type", Qt::CaseInsensitive) == 0)
		{
			DecorationType type = parts[1].trimmed();
			if (!para->DecorationTypes.contains(type))
			{
				qWarning("%s is not included", type.toStdString().c_str());
				continue;
			}			
			
			QString path;
			FurnitureType support_type;
			QVector<DecorationLocationType> location_types;
			float scale;
			QVector3D translate;
			for (size_t i = 0; i < 5; i++)
			{
				QByteArray inner_line = file->readLine();
				QString inner_str(inner_line);
				QStringList inner_parts = inner_str.split('=', QString::SkipEmptyParts);
				if (inner_parts.size() == 0)
					continue;
				if (QStrCmp(inner_parts[0], "Path"))
					path = inner_parts[1].trimmed();
				if (QStrCmp(inner_parts[0], "Support"))
					support_type = inner_parts[1].trimmed();				
				if (QStrCmp(inner_parts[0], "Location"))
					location_types = ParseDecorationLocationTypes(inner_parts[1]);
				if (QStrCmp(inner_parts[0], "Scale"))
					scale = QStr2Float(inner_parts[1]);
				if (QStrCmp(inner_parts[0], "Translate"))
					translate = Str2Vec3D(inner_parts[1]);
			}		
			if (para->FurnitureTypes.contains(support_type))
			{
				DecorationModel* model = new DecorationModel(support_type, type, location_types, scale, translate, path);
				decoration_models.push_back(model);
			}			
		}
	}
	file->close();
	delete file;
	return decoration_models;
}

QList<DecorationModel*> Utility::ParseDecorationModelsByType(QString& type)
{
	QList<DecorationModel*> decoration_models;
	Parameter *para = Parameter::GetParameterInstance();
	Assets *assets = Assets::GetAssetsInstance();
	
	if (!para->DecorationTypes.contains(type))
	{
		return decoration_models;
	}	

	QString path = Parameter::GetParameterInstance()->DatasetPath
		+ "decoration/" + type;
	QDir directory(path);
	if (!directory.exists())
		qWarning("decoration path %s does not exist",path.toStdString().c_str());
		
	QStringList names;
	QFileInfoList list = directory.entryInfoList();
	for (int i = 2; i<list.size(); i++)
	{
		QFileInfo fileInfo = list.at(i);
		if (fileInfo.isDir())
		{
			names.push_back(fileInfo.fileName());
		}
	}

	for (size_t i = 0; i < names.size(); i++)
	{
		float scale = assets->DecorationScales.contains(type) ? assets->DecorationScales[type] : 1.0f;
		DecorationModel* model = new DecorationModel(type, scale, names[i]);
		decoration_models.push_back(model);		
	}
	return decoration_models;
}

float Utility::QStr2Float(QString &str)
{
	return str.trimmed().toFloat();
}

float Utility::QStr2Int(QString& str)
{
	return str.trimmed().toInt();
}

bool Utility::QStr2Bool(QString& str)
{
	if (QStrCmp(str, "true"))
		return true;
	else if (QStrCmp(str, "false"))
		return false;
	else
	{
		qWarning("Wrong parameter at DrawboundingBox");
		return false;
	}
}

bool Utility::QStrIsImagePath(QString& str)
{
	return (str.contains(".jpg", Qt::CaseInsensitive)
		|| str.contains(".png", Qt::CaseInsensitive)
		|| str.contains(".jpeg", Qt::CaseInsensitive));
	
}

bool Utility::QStrCmp(QString& str1, char* str2)
{
	QString *qstr2 = new QString(str2);
	if (str1.trimmed().compare(qstr2->trimmed(),Qt::CaseInsensitive) == 0)
	{
		delete qstr2;
		return true;
	}
	else
	{
		delete qstr2;
		return false;
	}
}



QString Utility::GetFurnitureModelPath(QString& type, QString& name)
{
	
	QString result = Parameter::GetParameterInstance()->DatasetPath
		+ "furniture/" + type + "/" + name + "/model.obj";
	return result;
}

QString Utility::GetDecorationModelPath(QString& type, QString& name)
{
	QString result = Parameter::GetParameterInstance()->DatasetPath
		+ "decoration/" + type + "/" + name + "/model.obj";
	return result;
}

QString Utility::GetDecorationModelPath(QString& type)
{
	QString path = Parameter::GetParameterInstance()->DatasetPath
		+ "decoration/" + type;
	QDir directory(path);
	if (!directory.exists())
		return 0;
	QStringList names;
	QFileInfoList list = directory.entryInfoList();
	for (int i = 2; i<list.size(); i++)
	{
		QFileInfo fileInfo = list.at(i);
		if (fileInfo.isDir())
		{
			names.push_back(fileInfo.fileName());			
		}		
	}	
	qsrand(time(NULL));
	// random
	int r = qrand() % names.size();
	QString name = names[r];	
	return path + "/" + name + "/model.obj";
}

QVector<Light*> Utility::ParseLights()
{
	Parameter *para = Parameter::GetParameterInstance();
	QVector<Light*> lights;
	QFile *file = new QFile(para->LightDir);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + para->LightDir.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		if (str.contains("Directional light", Qt::CaseInsensitive))
		{
			QVector3D direction, ambient, diffuse, specular;
			for (size_t i = 0; i < 4; i++)
			{
				QByteArray inner_line = file->readLine();
				QString inner_str(inner_line);
				QStringList parts = inner_str.split('=', QString::SkipEmptyParts);
				if (parts.size() == 0)
					continue;
				if (parts[0].compare("Dir", Qt::CaseInsensitive) == 0)
				{
					direction = Utility::Str2Vec3D(parts[1]);
				}
				if (parts[0].compare("Ambient", Qt::CaseInsensitive) == 0)
				{
					ambient = Utility::Str2Vec3D(parts[1]);
				}
				if (parts[0].compare("Diffuse", Qt::CaseInsensitive) == 0)
				{
					diffuse = Utility::Str2Vec3D(parts[1]);
				}
				if (parts[0].compare("Specular", Qt::CaseInsensitive) == 0)
				{
					specular = Utility::Str2Vec3D(parts[1]);
				}
			}
			lights.push_back(new DirectionLight(direction, ambient, diffuse, specular));
		}
		if (str.contains("Point Light", Qt::CaseInsensitive))
		{
			int id = 0;
			QVector3D position, ambient, diffuse, specular;
			float constant = 1, linear = 0.09f, quadratic = 0.032f;
			for (size_t i = 0; i < 8; i++)
			{
				QByteArray inner_line = file->readLine();
				QString inner_str(inner_line);
				QStringList parts = inner_str.split('=', QString::SkipEmptyParts);
				if (parts.size() == 0)
					continue;
				if (parts[0].compare("Id", Qt::CaseInsensitive) == 0)
				{
					id = parts[1].toInt();
				}
				if (parts[0].compare("Position", Qt::CaseInsensitive) == 0)
				{
					position = Utility::Str2Vec3D(parts[1]);
				}
				if (parts[0].compare("Ambient", Qt::CaseInsensitive) == 0)
				{
					ambient = Utility::Str2Vec3D(parts[1]);
				}
				if (parts[0].compare("Diffuse", Qt::CaseInsensitive) == 0)
				{
					diffuse = Utility::Str2Vec3D(parts[1]);
				}
				if (parts[0].compare("Specular", Qt::CaseInsensitive) == 0)
				{
					specular = Utility::Str2Vec3D(parts[1]);
				}
				if (parts[0].compare("Constant", Qt::CaseInsensitive) == 0)
				{
					constant = parts[1].toFloat();
				}
				if (parts[0].compare("Linear", Qt::CaseInsensitive) == 0)
				{
					linear = parts[1].toFloat();
				}
				if (parts[0].compare("Quadratic", Qt::CaseInsensitive) == 0)
				{
					quadratic = parts[1].toFloat();
				}
			}
			lights.push_back(new PointLight(id, position, ambient, diffuse, specular, linear, quadratic, constant));
		}
	}
	file->close();
	delete file;

	return lights;
}

QVector<QString> Utility::ParseStringFromFile(QString & path)
{
	QVector<QString> strings;
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line.trimmed());
		strings.push_back(str);
	}
	file->close();
	delete file;
	return strings;
}

QVector<QString> Utility::QStr2StrVector(QString types)
{
	QVector<QString> result;
	QStringList parts = types.split(' ', QString::SkipEmptyParts);
	for (size_t i = 0; i < parts.size(); i++)
	{
		result.push_back(parts[i].trimmed());
	}
	return result;
}

vector<vector<int>> Utility::getCnm(vector<int> indices,int k)
{
	vector<vector<int>> results;
	size_t n = indices.size();
	//int values[] = { 1, 2, 3, 4, 5, 6, 7 };
	//vector<int> elements(n,0);
	/*for (size_t i = 0; i < k; i++)
	{
		elements[i] = 1;
	}*/
	//std::vector<int> selectors(elements, elements + n);
	int count = 0;
	do
	{
		vector<int> cur_comp;
		//std::cout << ++count << ": ";
		for (size_t i = 0; i < n; ++i)
		{
			//if (elements[i])
			//{
				cur_comp.push_back(indices[i]);
				//std::cout << values[i] << ", ";
			//}
		}
		results.push_back(cur_comp);
		//std::cout << std::endl;
	} while (next_permutation(indices.begin(), indices.end()));
	return results;
}

QStringList Utility::GetFileNames(QString& path)
{
	QStringList names;
	QDir directory(path);
	if (!directory.exists())
		qWarning("%s does not exist", path.toStdString().c_str());
	QFileInfoList namelist = directory.entryInfoList();
	for (int i = 2; i<namelist.size(); i++)
	{
		QFileInfo fileInfo = namelist.at(i);
		if (fileInfo.isFile())
		{
			names.push_back(fileInfo.fileName());
		}
	}
	return names;
}

QMap<QString, QVector3D> Utility::ParseColorsFromFile(QString& path)
{
	Parameter *para = Parameter::GetParameterInstance();
	QMap<QString, QVector3D> colors;
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split('=', QString::SkipEmptyParts);
		if (parts.size() < 2) // skip blank line
			continue;

		QString key = parts[0].trimmed();
		QVector3D color = Str2Vec3D(parts[1]) / 255.0;
		if (!colors.contains(key))
		{
			colors[key] = color;
		}
	}
	file->close();
	delete file;
	return colors;
}

QMap<FurnitureType, QVector<QString>> Utility::ParseMaterialMapFromFile(QString& path)
{
	Parameter *para = Parameter::GetParameterInstance();
	QMap<FurnitureType, QVector<QString>> materialColors;
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split('=', QString::SkipEmptyParts);
		if (parts.size() < 2) // skip blank line
			continue;
		QString key = parts[0].trimmed();
		QVector<QString> value;
		value = QStr2StrVector(parts[1]);		
		if (!materialColors.contains(key))
		{
			materialColors[key] = value;
		}
	}
	file->close();
	delete file;
	return materialColors;
}

QMap<QString, float> Utility::ParseQStrNameAndFloatValue(QString& path)
{
	Parameter *para = Parameter::GetParameterInstance();
	QMap<QString, float> orders;
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split(' ', QString::SkipEmptyParts);
		if (parts.size() < 2) // skip blank line
			continue;
		QString decorationType = parts[0].trimmed();
		double order = parts[1].trimmed().toFloat();		
		if (!orders.contains(decorationType) && para->DecorationTypes.contains(decorationType))
		{
			orders[decorationType] = order;
		}
	}
	file->close();
	delete file;
	return orders;

}

QMap<FurnitureType, QMap<QString, ColorPalette*>> Utility::ParseFurnitureTextureColors(QString& path)
{
	QMap<FurnitureType, QMap<QString, ColorPalette*>> furniture_texture_colors;
	QDir directory(path);
	if (!directory.exists())
	{
		qWarning("%s does not exist", path.toStdString().c_str());
		return furniture_texture_colors;
	}
	QStringList names;
	QFileInfoList list = directory.entryInfoList();
	for (int i = 2; i<list.size(); i++)
	{
		QFileInfo fileInfo = list.at(i);
		if (fileInfo.isDir())
		{
			names.push_back(fileInfo.fileName());
		}
	}
	QVector<FurnitureType> furniture_types = Parameter::GetParameterInstance()->FurnitureTypes;
	for (size_t i = 0; i < names.size(); i++)
	{
		if (furniture_types.contains(names[i]))
		{
			QMap<QString, ColorPalette*> color_map;

			QFile *file = new QFile(path + names[i] + "/5color.txt");
			if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
				std::cout << "Can't open file " + path.toStdString() << endl;
			while (!file->atEnd())
			{
				QByteArray line = file->readLine();
				QString str(line);
				QStringList parts = str.split(' ', QString::SkipEmptyParts);
				if (parts.size() != 16)
				{
					qWarning("Wrong texture color palette format.");
					continue;
				}
				QString key = parts[0];
				QVector<QColor> colors;
				for (size_t j = 1; j < parts.size(); j += 3)
				{
					colors.push_back(QColor(QStr2Int(parts[j]), QStr2Int(parts[j + 1]), QStr2Int(parts[j + 2])));				
				}
				color_map[key] = new ColorPalette(colors);
			}
			furniture_texture_colors[names[i]] = color_map;
		}
	}
	return furniture_texture_colors;
}

Texture* Utility::GetNearestColorTexture(QString& ft, ColorPalette* cp)
{
	Assets *assets = Assets::GetAssetsInstance();
	auto all_texture_colors = assets->FurnitureTextureColors;
	if (!all_texture_colors.contains(ft))
	{
		std::cout << ft.toStdString() << " does not have textures.\n";
		return nullptr;
	}
	double min_distance = INT_MAX;
	QString result_texture_name;
	auto texture_colors = all_texture_colors[ft];
	QMapIterator<QString,ColorPalette*> it(texture_colors);	
	while (it.hasNext())
	{
		it.next();
		double distance = ColorPalette::GetColorPaletteDistance(cp, it.value());
		if (min_distance > distance)
		{
			min_distance = distance;
			result_texture_name = it.key();
		}		
	}	
	QString path = Parameter::GetParameterInstance()->TexturePath + ft + "/" + result_texture_name;
	return assets->GetTexture(path);
}

Texture* Utility::GetNearestColorTexture(QString& ft, QColor& cp)
{
	Assets *assets = Assets::GetAssetsInstance();
	auto all_texture_colors = assets->FurnitureTextureColors;
	if (!all_texture_colors.contains(ft))
	{
		std::cout << ft.toStdString() << " does not have textures.\n";
		return nullptr;
	}
	double min_distance = INT_MAX;
	QString result_texture_name;
	auto texture_colors = all_texture_colors[ft];
	QVector<QColor> colors;
	colors.push_back(cp);
	ColorPalette *query_color = new ColorPalette(colors);
	//QMapIterator<QString, ColorPalette*> it(texture_colors);
	for (auto it: texture_colors.toStdMap())
	{
		double distance = ColorPalette::GetColorPaletteDistance(query_color, it.second, true);
		if (min_distance > distance)
		{
			min_distance = distance;
			result_texture_name = it.first;
		}
	}
	//while (it.hasNext())
	//{
	//	it.next();		
	//	//double distance = ColorPalette::GetColorDistance(cp, it.value()->Colors[0]);
	//	ColorPalette* cur_cp = it.value();
	//	double distance = ColorPalette::GetColorPaletteDistance(query_color, cur_cp,true);
	//	if (min_distance > distance)
	//	{
	//		min_distance = distance;
	//		result_texture_name = it.key();
	//	}
	//}
	delete query_color;
	QString path = Parameter::GetParameterInstance()->TexturePath + ft + "/" + result_texture_name;
	return assets->GetTexture(path);
}

Material* Utility::GetMaterialFromSingleTexture(QString path)
{
	QOpenGLTexture *gl_texture;
	gl_texture = new QOpenGLTexture(QImage(path).mirrored());
	//texture->setAutoMipMapGenerationEnabled(true);
	gl_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	gl_texture->setMagnificationFilter(QOpenGLTexture::Linear);
	gl_texture->setWrapMode(QOpenGLTexture::MirroredRepeat);

	Texture *texture = new Texture();
	texture->texture = gl_texture;
	texture->type = DiffuseTexture;
	texture->fullpath = path;
	QVector<Texture*> tmptextures;
	tmptextures.push_back(texture);

	Material* result = new Material();
	result->Diffuse = new MaterialElement(tmptextures);
	result->HasTexture = true;
	return result;	
}

Material* Utility::GetMaterialFromSingleColor(QVector3D &diffuse_color)
{
	Material* result = new Material();
	result->Diffuse = new MaterialElement(diffuse_color);
	return result;
}

Material* Utility::GetMaterialFromString(QString& material)
{
	if (Assets::GetAssetsInstance()->GetColors().contains(material)) // it is a color
	{
		return GetMaterialFromSingleColor(Assets::GetAssetsInstance()->GetColorByName(material));
	}
	else // it is an image path
	{
		return GetMaterialFromSingleTexture(material);
	}
}

QVector<FurnitureLocationType> Utility::ParseLocationTypes(QString types)
{
	QVector<FurnitureLocationType> result;
	QStringList type_list = types.split(' ', QString::SkipEmptyParts);
	for (size_t i = 0; i < type_list.size(); i++)
	{
		if (QStrCmp(type_list[i], "Bottom"))
			result.push_back(FTBottom);
		else if (QStrCmp(type_list[i], "Up"))
			result.push_back(FTUp);
		else if (QStrCmp(type_list[i], "Left"))
			result.push_back(FTLeft);
		else if (QStrCmp(type_list[i], "Right"))
			result.push_back(FTRight);
		else if (QStrCmp(type_list[i], "Back"))
			result.push_back(FTBack);
		else if (QStrCmp(type_list[i], "Front"))
			result.push_back(FTFront);
		else
			qWarning("Invalid furniture location: %s ", type_list[i].toStdString().c_str());

	}
	return result;
}

QVector<DecorationLocationType> Utility::ParseDecorationLocationTypes(QString types)
{
	QVector<DecorationLocationType> result;
	QStringList type_list = types.split(' ', QString::SkipEmptyParts);
	for (size_t i = 0; i < type_list.size(); i++)
	{		
		if (QStrCmp(type_list[i], "NotSet"))
			result.push_back(NotSet);
		else if (QStrCmp(type_list[i], "Center"))
			result.push_back(Center);
		else if (QStrCmp(type_list[i], "Left"))
			result.push_back(Left);
		else if (QStrCmp(type_list[i], "Right"))
			result.push_back(Right);
		else if (QStrCmp(type_list[i], "Back"))
			result.push_back(Back);
		else if (QStrCmp(type_list[i], "Front"))
			result.push_back(Front);
		else
			qWarning("Invalid decoration location: %s ", type_list[i].toStdString().c_str());

	}
	return result;
}

float Utility::GetCrossArea(QVector3D& rec1_v1, QVector3D& rec1_v2, QVector3D& rec2_v1, QVector3D& rec2_v2)
{
	float rec1_minx = qMin(rec1_v1.x(), rec1_v2.x());
	float rec1_maxx = qMax(rec1_v1.x(), rec1_v2.x());
	float rec1_minz = qMin(rec1_v1.z(), rec1_v2.z());
	float rec1_maxz = qMax(rec1_v1.z(), rec1_v2.z());
	float rec2_minx = qMin(rec2_v1.x(), rec2_v2.x());
	float rec2_maxx = qMax(rec2_v1.x(), rec2_v2.x());
	float rec2_minz = qMin(rec2_v1.z(), rec2_v2.z());
	float rec2_maxz = qMax(rec2_v1.z(), rec2_v2.z());

	float min_x = qMax(rec1_minx, rec2_minx);
	float max_x = qMin(rec1_maxx, rec2_maxx);
	float min_z = qMax(rec1_minz, rec2_minz);	
	float max_z = qMin(rec1_maxz, rec2_maxz);

	if (max_x > min_x && max_z > min_z)	
		return (max_x - min_x)*(max_z - min_z);	
	else // non-intersect
		return 0;	
}

QMap<QString, ColorPalette*> Utility::ReadImageFurnitureInfo(QString& path)
{
	QMap<FurnitureType, ColorPalette*> map;	
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		if (str.contains("Furniture Color",Qt::CaseInsensitive))
		{
			while(!file->atEnd())
			{
				QByteArray inner_line = file->readLine();
				QString inner_str(inner_line);
				QStringList inner_parts = inner_str.split('=', QString::SkipEmptyParts);
				if (inner_parts.size() < 2)
					break;
				FurnitureType type = inner_parts[0].trimmed();
				QStringList colors = inner_parts[1].split(' ', QString::SkipEmptyParts);
				QVector<QColor> colorvec;
				for (size_t i = 0; i < colors.size()/3; i++)
				{
					int r = QStr2Int(colors[i * 3 + 0]);
					int g = QStr2Int(colors[i * 3 + 1]);
					int b = QStr2Int(colors[i * 3 + 2]);
					colorvec.push_back(QColor(r, g, b));					
				}
				if (!map.contains(type))
				{
					//map[type] = new ColorPalette(colorvec);
					map[type] = new ColorPalette(colorvec,path);
				}				
			}
		}
		else if (str.contains("Decorations",Qt::CaseInsensitive))
		{
			break;
		}		
	}
	file->close();
	delete file;	
	return map;
}

QList<QPair<QString, QPair<QString, QVector<DecorationLocationType>>>> Utility::ReadImageDecorationInfo(QString& path)
{
	QList<QPair<QString, QPair<QString, QVector<DecorationLocationType>>>> list;
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		if (str.contains("Decorations", Qt::CaseInsensitive))
		{
			while (!file->atEnd())
			{
				QByteArray inner_line = file->readLine();
				QString inner_str(inner_line);
				QStringList inner_parts = inner_str.split('=', QString::SkipEmptyParts);
				if (inner_parts.size() < 2)
					break;
				DecorationType type = inner_parts[0].trimmed();				
				QStringList decoration_info = inner_parts[1].split('|', QString::SkipEmptyParts);
				FurnitureType support_furniture = decoration_info[0].trimmed();
				// 床上物品的摆放转换成床单
				if (support_furniture == "Bed")
				{
					support_furniture = "BedSheet";
				}
				if (support_furniture == "Floor")
				{
					support_furniture = "FloorProxy";
				}
				QVector<DecorationLocationType> decoration_loc = ParseDecorationLocationTypes(decoration_info[1]);
				auto decorationInfoPair = QPair<FurnitureType, QVector<DecorationLocationType>>(support_furniture, decoration_loc);
				list.push_back(QPair<QString, QPair<QString, QVector<DecorationLocationType>>>(type, decorationInfoPair));
			}
		}		
	}
	file->close();
	delete file;
	return list;
}

Camera* Utility::ReadCamera(QString& path)
{	
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;	
	QVector3D pos, up= QVector3D(0.0f, 1.0f, 0.0f);
	float yaw = 0, pitch = 0, zoom = 45.0f;

	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
	
		QStringList parts = str.split('=', QString::SkipEmptyParts);
		if (parts[0].compare("Position",Qt::CaseInsensitive) == 0)
		{
			pos = Str2Vec3D(parts[1]);
		}
		if (parts[0].compare("Up", Qt::CaseInsensitive) == 0)
		{
			up = Str2Vec3D(parts[1]);
		}
		if (parts[0].compare("Yaw", Qt::CaseInsensitive) == 0)
		{
			yaw = QStr2Float(parts[1]);
		}
		if (parts[0].compare("Pitch", Qt::CaseInsensitive) == 0)
		{
			pitch = QStr2Float(parts[1]);
		}
		if (parts[0].compare("Zoom", Qt::CaseInsensitive) == 0)
		{
			zoom = QStr2Float(parts[1]);
		}		
	}
	file->close();
	delete file;
	return new Camera(pos, up, yaw, pitch, zoom);
}
