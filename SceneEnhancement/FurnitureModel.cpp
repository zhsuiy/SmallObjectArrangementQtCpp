#include "FurnitureModel.h"
#include "Parameter.h"
#include "RecolorImage.h"
#include "SmallObjectArrange.h"
#include "com_retriever.h"
#include <vector>

FurnitureModel::FurnitureModel()
{
	this->IsShowTexture = false;
}

FurnitureModel::FurnitureModel(QString type, QString name, QVector3D translate, QVector3D rotate,
							   QVector<FurnitureLocationType> locationTypes, float scale)
				:Model(Utility::GetFurnitureModelPath(type,name),translate,rotate,scale)
{	
	this->Type = type;
	this->LocationTypes = locationTypes;
	this->IsShowTexture = false;
	updateTranslation();
	updateFrontDirection(rotate);	
	this->OrderMaterialByMeshArea();
	this->UpdateMeshMaterials();
	this->SetModelMatrix();
	
	this->UpdateBoundingBoxWorldCoordinates();
	this->DetectSupportRegions(); 
}

void FurnitureModel::SetModelMatrix()
{
	modelMatrix.setToIdentity();
	modelMatrix.translate(m_translate);
	modelMatrix.scale(m_scale);
	modelMatrix.rotate(m_rotate.x(), 1, 0, 0);
	modelMatrix.rotate(m_rotate.y(), 0, 1, 0);
	modelMatrix.rotate(m_rotate.z(), 0, 0, 1);
}


void FurnitureModel::DetectSupportRegions()
{
	QMap<float, QVector<QVector3D>> all_support_vertices;
	auto para = Parameter::GetParameterInstance();
	for (size_t i = 0; i < this->meshes.size(); i++)
	{
		Mesh *mesh = this->meshes[i];
		for (size_t j = 0; j < mesh->Vertices.size(); j++)
		{
			//  0. test for up direction
			if (abs(mesh->Vertices[j].normal().x()) < 1e-1 && abs(abs(mesh->Vertices[j].normal().y()) - 1) <= 1e-1 
				&& abs(mesh->Vertices[j].normal().z()) <= 4e-1)
			{
				// 1. group vertices according to vertex y position
				float height = mesh->Vertices[j].position().y();
				if (!all_support_vertices.contains(height))
				{			
					bool flag = false;
					for (size_t k = 0; k < all_support_vertices.keys().size(); k++)
					{
						// 说明这两个高度差别很小
						if (abs(all_support_vertices.keys()[k] - height) < 0.08)
						{
							if (all_support_vertices.keys()[k] < height) // 替换key
							{
								auto ver = all_support_vertices[all_support_vertices.keys()[k]];
								ver.push_back(mesh->Vertices[j].position());
								all_support_vertices.remove(all_support_vertices.keys()[k]);
								all_support_vertices[height] = ver;
							}
							else
							{
								all_support_vertices[all_support_vertices.keys()[k]].push_back(mesh->Vertices[j].position());								
							}
							flag = true;
							break;							
						}
					}
					if (!flag)
					{
						QVector<QVector3D> vertices;
						vertices.push_back(mesh->Vertices[j].position());
						all_support_vertices[height] = vertices;
					}										
				}
				else
				{
					all_support_vertices[height].push_back(mesh->Vertices[j].position());
				}				
			}
		}
	}

	// 2. sort support layer according to vertex height	
	QMapIterator<float, QVector<QVector3D>> it(all_support_vertices);
	QVector<QPair<float, QVector<QVector3D>>> support_layers;			
	while (it.hasNext())
	{
		it.next();
		support_layers.push_front(QPair<float, QVector<QVector3D>>(it.key(), it.value()));					
	}
	
	// 3. filter out layers that are too small or too close to last layer
	float bbwidth = this->boundingBox->Width();
	float bbdepth = this->boundingBox->Depth();
	float bbheight = this->boundingBox->Height();
	int support_num = 1; //其他家具默认只有一个摆放区域
	if (para->MultiLayerFurnitures.contains(this->Type))
	{
		//support_num = 10; // 4层书架
		support_num = 3; // 后面的例子
	}
	//if (this->Type.compare("Cabinet",Qt::CaseInsensitive) == 0
	//	|| this->Type.compare("SideTable", Qt::CaseInsensitive) == 0) // 橱柜单独处理,可以有多层
	//{
	//	support_num = 10;
	//}
	
	float last_height = this->boundingBox->RightUpFront().y() + bbheight / support_num;

	for (size_t i = 0; i < support_layers.size(); i++)
	{
		float min_x = INT_MAX, max_x = -INT_MAX, min_z = INT_MAX, max_z = -INT_MAX;
		float cur_height = support_layers[i].first;
		for (size_t j = 0; j < support_layers[i].second.size(); j++)
		{
			QVector3D &v = support_layers[i].second[j];
			if (min_x > v.x())			
				min_x = v.x();			
			if (max_x < v.x())
				max_x = v.x();
			if (min_z > v.z())
				min_z = v.z();
			if (max_z < v.z())
				max_z = v.z();			
		}
		// used to be if (max_x - min_x >= bbwidth/4 && max_z - min_z >= bbdepth/4
		if (max_x - min_x >= bbwidth/4.0 && max_z - min_z >= bbdepth/4.0	// filter out small layers
			&&  abs(last_height - cur_height) >= (bbheight / (support_num + 1))) // two layers should not too close in height
		{
			this->support_regions.push_back(new SupportRegion(min_x, max_x, min_z, max_z, cur_height, modelMatrix));			
			last_height = cur_height;
			if (this->support_regions.size() >= support_num)
			{
				break;
			}
		}
	}
}

QVector3D FurnitureModel::GetRelativePosition(DecorationModel* model)
{
	QVector3D translate;
	
	return translate;
}

// 有待修改 - 应该考虑家具的朝向
void FurnitureModel::updateTranslation()
{
	Assets *assets = Assets::GetAssetsInstance();
	for (size_t i = 0; i < LocationTypes.size(); i++)
	{		
		switch (LocationTypes[i])
		{
		case FTBottom:
			this->m_translate.setY(boundingBox->Height() / 2 * m_scale + 0.05f);		
			break;
		case FTUp:
			this->m_translate.setY(assets->RoomHeight - boundingBox->Height()/2 * m_scale - 0.01f);
			break;
		case FTLeft:
			if (m_rotate.y() / 90 == 1 || m_rotate.y() / 90 == 3) // 旋转了			
				this->m_translate.setX(boundingBox->Depth() / 2.0 * m_scale + 0.01f);
			else
				this->m_translate.setX(boundingBox->Width() / 2 * m_scale + 0.01f);
			break;
		case FTRight:
			if (m_rotate.y() / 90 == 1 || m_rotate.y() / 90 == 3) // 旋转了			
				this->m_translate.setX(assets->RoomWidth - boundingBox->Depth() / 2.0 * m_scale + 0.01f);
			else
				this->m_translate.setX(assets->RoomWidth - boundingBox->Width() / 2 * m_scale - 0.01f);
			break;
		case FTFront:
			if (m_rotate.y() / 90 == 1 || m_rotate.y() / 90 == 3) // 旋转了			
				this->m_translate.setZ(assets->RoomDepth - boundingBox->Width() / 2.0 * m_scale + 0.01f);
			else
				this->m_translate.setZ(assets->RoomDepth - boundingBox->Depth() / 2 * m_scale - 0.01f);
			break;
		case FTBack:
			if (m_rotate.y() / 90 == 1 || m_rotate.y() / 90 == 3) // 旋转了			
				this->m_translate.setZ(boundingBox->Width() / 2.0 * m_scale + 0.01f);
			else // 没有旋转，如墙壁
				this->m_translate.setZ(boundingBox->Depth() / 2.0 * m_scale + 0.01f);			
			break;
		default:
			break;
		}
	}	
}

void FurnitureModel::updateFrontDirection(QVector3D& rotate)
{
	float x = rotate.x();
	float y = rotate.y();
	float z = rotate.z();

	if (x==0 && y==0 && z==0)	
		FurnitureFrontDirection = XPos;	
	else if(x == 0 && y == 90 && z == 0 )
		FurnitureFrontDirection = ZPos;
	else if (x == 0 && y == 180 && z==0)
		FurnitureFrontDirection = XNeg;
	else if (x == 0 && y==270 && z==0)
		FurnitureFrontDirection = ZNeg;
	else
		FurnitureFrontDirection = Invalid;
}

int compare(const QPair<QString, float> &a1, const QPair<QString, float> &a2)
{
	return a1.second > a2.second;
}

void FurnitureModel::UpdateDecorationLayoutActiveLearning(SmallObjectArrange * arranger)
{	
	if (decoration_models.size() == 0)
	{
		return;
	}
	auto para = Parameter::GetParameterInstance();

	if (support_regions.size() == 1) // 单层物体
	{
		SupportRegion *support_region = this->support_regions[0];

		// Step 1. 初步的filter,去掉面积过大的物体
		QVector<DecorationModel*> tmp_models;
		float sum_area = 0;
		float support_area = support_region->Width * support_region->Depth;
		for (size_t i = 0; i < decoration_models.size(); i++)
		{
			DecorationModel* model = decoration_models[i];
			float area = model->boundingBox->Depth()*model->GetScale()*model->boundingBox->Width()*model->GetScale();
			if (sum_area + area < support_area*para->SupportRegionPercent) //面积比80%的support region小
			{
				sum_area += area;
				tmp_models.push_back(model);
			}
			else
			{
				// remove this model from rendering list
				model->IsAssigned = false;
			}
		}
		decoration_models = tmp_models;

		// Step 2. 单层的	
		// 摆得下		
		double F = support_region->ArrangeDecorationModels(this, decoration_models,arranger);
		std::cout << this->Type.toStdString() << " Decoration Score: " << F << std::endl;
	}
	else// 多层的
	{
		// 先把IsAssigned置0
		for (size_t i = 0; i < decoration_models.size(); i++)
		{
			decoration_models[i]->IsAssigned = false;
		}

		QVector<QString> dec_cats;
		for (size_t i = 0; i < decoration_models.size(); i++)
		{
			dec_cats.push_back(decoration_models[i]->Type);
		}
		// calculate cost
		float height_cost = 10000;
		// from top to down
		int n_layers = support_regions.size();		
		int m = decoration_models.size();
		QVector<QVector<QString>> dec_each_layer;
		vector<vector<int>> dec_each_layer_index, best_each_layer_index;		
		int init = Parameter::GetParameterInstance()->AllowTupFurnitures.contains(this->Type) ? 0 : 1;
		auto all_combs_layers = getIndexPerLayer(m, (n_layers-init));
				
		std::cout << "Assigning small objects to each layer\n";

		for (size_t i = 0; i < all_combs_layers.size(); i++)
		{
			dec_each_layer_index = all_combs_layers[i];
			dec_each_layer.clear();
			for (size_t j = 0; j < dec_each_layer_index.size(); j++)
			{
				QVector<QString> cur;
				for (size_t k = 0; k < dec_each_layer_index[j].size(); k++)
				{
					cur.push_back(dec_cats[dec_each_layer_index[j][k]]);
				}
				dec_each_layer.push_back(cur);
			}
			float cost = getHeightCost(dec_each_layer, arranger);
			if (cost < height_cost)
			{
				height_cost = cost;
				best_each_layer_index = dec_each_layer_index;
			}
			if (i % 10 == 0)
				std::cout << ".";			
		}	

		double F = 0.0;
		
		// 确保每层都有
		int m_added = 0;
		std::cout << "\nArranging small objects layer by layer\n";
		int cur_layer = 0; // to extract small objects
		for (size_t i = init; i < n_layers; i++) //for (size_t i = 0; i < n; i++)  when top layer is not allowed
		{
			std::cout << "Arrange " << i << "th" << " layer...\n";
			SupportRegion *support_region = this->support_regions[i];
			QVector<DecorationModel*> tmp_models;
			if (cur_layer >= best_each_layer_index.size())
				break;
			auto dec_indices = best_each_layer_index[cur_layer++];		
			for (size_t j = 0; j < dec_indices.size(); j++)
			{
				DecorationModel* model = decoration_models[dec_indices[j]];
				tmp_models.push_back(model);
				model->IsAssigned = true;
			}
			F += support_region->ArrangeDecorationModels(this, tmp_models, arranger);
			std::cout << "\nLayer " << i << " score: " << F << std::endl;
		}
		std::cout << this->Type.toStdString() << " Decoration Score: " << F << std::endl;
	}
}

float FurnitureModel::getHeightCost(QVector<QVector<QString>>& cat_per_layer, SmallObjectArrange * arranger)
{

	auto height_pref = arranger->GetHeightHigherProb();
	auto height_equal = arranger->GetHeightEqualProb();
	auto cat_index_map = arranger->GetCatIndexMapping();
	auto height_user_pref_index_pair = arranger->GetUserPrefHeightIndexPair();
	
	float f = 0.0;
	QVector<QPair<QString, int>> cat_height_pair;
	for (size_t i = 0; i < cat_per_layer.size(); i++)
	{
		auto cats = cat_per_layer[i];
		for (size_t j = 0; j < cats.size(); j++)
		{
			cat_height_pair.push_back(qMakePair(cats[j], i));
		}
	}

	float norm_n = 0.0;
	for (size_t i = 0; i < cat_height_pair.size(); i++)
	{
		for (size_t j = 0; j < cat_height_pair.size(); j++)
		{
			auto cati = cat_height_pair[i].first;
			auto catj = cat_height_pair[j].first;
			auto layeri = cat_height_pair[i].second;
			auto layerj = cat_height_pair[j].second;
			if (cat_index_map.contains(cati) &&
				cat_index_map.contains(catj))
			{
				// index in active learning data
				auto index_i = cat_index_map[cati];
				auto index_j = cat_index_map[catj];

				bool is_user_index_flag = height_user_pref_index_pair.find(index_i) != height_user_pref_index_pair.end() &&
					height_user_pref_index_pair[index_i].find(index_j) != height_user_pref_index_pair[index_i].end() ||
					height_user_pref_index_pair.find(index_j) != height_user_pref_index_pair.end() &&
					height_user_pref_index_pair[index_j].find(index_i) != height_user_pref_index_pair[index_j].end();

				auto equal_ij = height_equal[index_i][index_j];
				auto height_higher_ij = height_pref[index_i][index_j];
				// not defined
				if (equal_ij == -1 || height_higher_ij == -1)
					continue;

				// equal cost:
				// record how the two objects are at different heights
				double equal_cost = layeri == layerj ? 0 : 1.0 / (1.0 + exp(-(abs(layeri - layerj))));
				equal_ij = is_user_index_flag ? 10 * equal_ij : equal_ij;
				f += equal_ij*equal_cost;
				// record how 
				// j should be back, i should be front
				// using depth_front_ij to punish how j is wrongly put in front of i
				double unequal_cost = layeri < layerj ? 0 : 1.0 / (1.0 + exp(-(layeri - layerj)));
				height_higher_ij = is_user_index_flag ? 10 * height_higher_ij : height_higher_ij;
				f += height_higher_ij* unequal_cost;
				norm_n++;
			}
		}
	}
	f /= norm_n;

	return f;
}

void FurnitureModel::OrderMaterialByMeshArea()
{
	QMap<QString, float> tmp;
	QList<QPair<QString, float>> materials;

	for (size_t i = 0; i < this->meshes.size(); i++)
	{
		QString materialName = this->meshes[i]->MeshMaterial->Name;
		tmp[materialName] += this->meshes[i]->GetArea();
	}

	QMapIterator<QString, float> it(tmp);
	while (it.hasNext())
	{
		it.next();
		materials.push_back(QPair<QString, float>(it.key(), it.value()));
	}

	qSort(materials.begin(), materials.end(), Utility::QPairSecondComparer());
	for (size_t i = 0; i < materials.size(); i++)
	{
		ordered_materials.push_back(material_assets[materials[i].first]);
	}
}

void FurnitureModel::updateTextureState()
{
	Parameter *para = Parameter::GetParameterInstance();
	for (size_t i = 0; i < this->ordered_materials.size(); i++)
	{
		
		if (IsShowTexture && ordered_materials[i]->HasTexture && para->FurnitureTypesUseTextures.contains(this->Type)) // use texture
		{
			ordered_materials[i]->Diffuse->UseMap = true;			
		}
		else
		{
			ordered_materials[i]->Diffuse->UseMap = false;		
		}
	}
}

void FurnitureModel::UpdateMeshMaterials()
{
	Assets *assets = Assets::GetAssetsInstance();
	if (assets->MaterialMap.contains(Type))
	{
		QVector<QString> colorNames = assets->MaterialMap[Type];
		int n = colorNames.size();
		for (size_t i = 0; i < this->ordered_materials.size(); i++)
		{
			QString color = colorNames[i%n];
			if (Utility::QStrIsImagePath(color))
			{
				ordered_materials[i]->Diffuse->UseMap = true;
				QOpenGLTexture *gl_texture;
				gl_texture = new QOpenGLTexture(QImage(color).mirrored());
				//texture->setAutoMipMapGenerationEnabled(true);
				gl_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
				gl_texture->setMagnificationFilter(QOpenGLTexture::Linear);
				gl_texture->setWrapMode(QOpenGLTexture::MirroredRepeat);

				Texture *texture = new Texture();
				texture->texture = gl_texture;
				texture->type = DiffuseTexture;
				texture->fullpath = color;
				QVector<Texture*> tmptextures;
				tmptextures.push_back(texture);
				ordered_materials[i]->Diffuse->Textures = tmptextures;
			}
			else
			{
				ordered_materials[i]->Diffuse->UseMap = false;
				ordered_materials[i]->Diffuse->Color = assets->GetColorByName(colorNames[i%n]);
			}
		}
	}
}

void FurnitureModel::UpdateMeshMaterials(ColorPalette* color_palette)
{
	Parameter *para = Parameter::GetParameterInstance();
	QVector<QColor> colors = color_palette->Colors;
	for (size_t i = 0; i < this->ordered_materials.size(); i++)
	{
		QColor color = colors[i%colors.size()];
		if (ordered_materials[i]->Diffuse->Textures.size() > 0)
		{
			if (this->Type == "WallPhoto" || this->Type == "TV")
			{
				QVector<Texture*> tmptextures;
				Texture *t = Utility::GetNearestColorTexture(this->Type, color);
				assert(t != nullptr);
				tmptextures.push_back(t);
				ordered_materials[i]->Diffuse->Textures = tmptextures;
			}
			else
			{
				int a = 1;
				auto cur_tex = ordered_materials[i]->Diffuse->Textures[0];

				//Generate recolored texture
				QString filename = cur_tex->fullpath;
				QOpenGLTexture *recolored_texture;
				QImage recoloredImage = Utility::RecolorQImage(filename, color);

				recolored_texture = new QOpenGLTexture(recoloredImage.mirrored());
				//texture->setAutoMipMapGenerationEnabled(true);
				recolored_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
				recolored_texture->setMagnificationFilter(QOpenGLTexture::Linear);
				recolored_texture->setWrapMode(QOpenGLTexture::Repeat);
				cur_tex->texture = recolored_texture;
			}			
		}		

		ordered_materials[i]->Diffuse->Color = QVector3D(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0);
	//}
	}
	updateTextureState();
}

void FurnitureModel::AddDecorationModel(DecorationModel* model)
{
	model->SupportModelType = this->Type;
	model->SupportModel = this;
	model->SetTranslation(this->GetTranslate());
	model->IsAssigned = true;
	decoration_models.push_back(model);	
}

void FurnitureModel::UpdateDecorationLayout()
{
	auto para = Parameter::GetParameterInstance();
	if (decoration_models.size() == 0)
	{
		return;
	}
	if (support_regions.size() == 1) // 单层物体
	{		
		SupportRegion *support_region = this->support_regions[0];

		// Step 1. 初步的filter,去掉面积过大的物体
		QVector<DecorationModel*> tmp_models;
		float sum_area = 0;
		float support_area = support_region->Width * support_region->Depth;
		for (size_t i = 0; i < decoration_models.size(); i++)
		{
			DecorationModel* model = decoration_models[i];
			float area = model->boundingBox->Depth()*model->GetScale()*model->boundingBox->Width()*model->GetScale();
			if (sum_area + area < support_area*para->SupportRegionPercent) //面积比80%的support region小
			{
				sum_area += area;
				tmp_models.push_back(model);
			}
			else
			{
				// remove this model from rendering list
				model->IsAssigned = false;
			}
		}
		decoration_models = tmp_models;

		// Step 2. 单层的	
		// 摆得下		
		double F = support_region->ArrangeDecorationModels(this, decoration_models);
		std::cout << this->Type.toStdString() << " Decoration Score: " << F << std::endl;
	}
	else// 多层的
	{
		// 先把IsAssigned置0
		for (size_t i = 0; i < decoration_models.size(); i++)
		{
			decoration_models[i]->IsAssigned = false;
		}		
		
		// 根据HeightOrder重新排序
		auto all_height_orders = Assets::GetAssetsInstance()->DecorationHOrders;
		QVector<QPair<int, double>> decorationheightorder;
		for (size_t i = 0; i < decoration_models.size(); i++)
		{
			if (all_height_orders.keys().contains(decoration_models[i]->Type))
				decorationheightorder.push_back(QPair<int, double>(i, all_height_orders[decoration_models[i]->Type]));
			else // 其他物体的高度优先级是0
				decorationheightorder.push_back(QPair<int, double>(i, 0));
		}
		qSort(decorationheightorder.begin(), decorationheightorder.end(), Utility::QPairSecondComparer());
		QVector<DecorationModel*> height_ordered_dec_list;
		for (size_t i = 0; i < decorationheightorder.size(); i++)
		{
			height_ordered_dec_list.push_back(decoration_models[decorationheightorder[i].first]);
		}
		decoration_models = height_ordered_dec_list;


		double F = 0.0;
		int n = support_regions.size();
		// 确保每层都有
		int m_added = 0;
		int init = Parameter::GetParameterInstance()->AllowTupFurnitures.contains(this->Type) ? 0 : 1;
		for (size_t i = init; i < n; i++) //for (size_t i = 0; i < n; i++)  when top layer is not allowed
		{
			SupportRegion *support_region = this->support_regions[i];
			QVector<DecorationModel*> tmp_models;
			float sum_area = 0;
			float support_area = support_region->Width * support_region->Depth;
			for (size_t j = 0; j < decoration_models.size(); j++)
			{
				DecorationModel* model = decoration_models[j];
				if (model->IsAssigned)
				{
					continue;
				}
				float area = model->boundingBox->Depth()*model->GetScale()*model->boundingBox->Width()*model->GetScale();
				float height = model->boundingBox->Height()*model->GetScale();
				if (sum_area + area < support_area*para->SupportRegionPercent) //面积比support region小
				//if (sum_area + area < support_area*0.7) //面积比support region小
				{
					if (i > 0) // 中间层要考虑高度差
					{
						// 两层之差
						float support_region_height = support_regions[i - 1]->Height - support_region->Height;
						if (height < support_region_height)
						{
							sum_area += area;
							tmp_models.push_back(model);
							m_added++;
							model->IsAssigned = true;
							if (tmp_models.size() >= para->EachSupportLayerMaxModelNum || (decoration_models.size() - m_added) <= (n-i-1))
							{
								break;
							}
						}
						else
						{
							model->IsAssigned = false;
						}
					}
					else
					{
						sum_area += area;
						tmp_models.push_back(model);
						m_added++;
						model->IsAssigned = true;
						if (/*tmp_models.size() >= 2 ||*/ (decoration_models.size() - m_added) <= (n - i))
						{
							break;
						}
					}
				}
				else
				{
					// remove this model from rendering list					
					model->IsAssigned = false;
				}
			}
			F += support_region->ArrangeDecorationModels(this, tmp_models);
		}
		std::cout << this->Type.toStdString() << " Decoration Score: " << F << std::endl;	
	}
}

void FurnitureModel::UpdateDecorationYAlignment()
{
	auto para = Parameter::GetParameterInstance();
	if (decoration_models.size() == 0)
	{
		return;
	}
	if (support_regions.size() == 1) // 单层物体
	{
		SupportRegion *support_region = this->support_regions[0];			
		support_region->AlignDecorationModelsY(this,decoration_models);
	}
	else// 多层的
	{
		// 暂不考虑
	}
}

// see ./config/DecorationModels_girl.txt
void FurnitureModel::UpdateDecorationLayoutWithConstraints()
{
	int layer = this->support_regions.size() - 1;
	SupportRegion *support_region;
	for (size_t i = 0; i < decoration_models.size(); i++)
	{
		support_region = this->support_regions[layer];
		DecorationModel *model = decoration_models[i];

		AdaptDecorationLocationType(model);
		float tx = 0, tz = 0;
		for (size_t j = 0; j < model->LocationTypes.size(); j++)
		{
			switch (model->LocationTypes[j])
			{
			case NotSet:
				tx = model->GetRelativeTranslate().x();
				tz = model->GetRelativeTranslate().z();
				break;
			/*case Center:
				tx = 0;
				tz = 0;*/
			case Left:
				tx = -(this->boundingBox->Width() / 2 * m_scale
					- (abs(this->boundingBox->WorldLeftBottomBack().x() - support_region->MinX))
					- model->boundingBox->Width() / 2 * model->GetScale());				
				break;
			case Right:
				tx = this->boundingBox->Width() / 2 * m_scale
					- abs(this->boundingBox->WorldRightUpFront().x() - support_region->MaxX)
					- model->boundingBox->Width() / 2 * model->GetScale();				
				break;
			case Back:
				tz = -(this->boundingBox->Depth() / 2 * m_scale
					- abs(this->boundingBox->WorldLeftBottomBack().z() - support_region->MinZ)
					- model->boundingBox->Depth() / 2 * model->GetScale());				
				break;
			case Front:
				tz = this->boundingBox->Depth() / 2 * m_scale
					- abs(this->boundingBox->WorldRightUpFront().z() - support_region->MaxZ)
					- model->boundingBox->Depth() / 2 * model->GetScale();				
				break;
			default:
				break;
			}
		}		
		AdaptTranslateAccord2FrontDirection(tx,tz); // 根据家具的朝向调整translate
		float ty = support_region->Height - this->m_translate.y()  + abs(model->boundingBox->LeftBottomBack().y())*model->GetScale();
		model->SetRelativeTranslate(tx, ty, tz);
		
		if (!support_region->TryPutDecorationModel(model)) // 如果一直放不上去，会卡在这里
		{
			if (layer == 0)
			{
				break;
			}
			i--; // replace;
			if (layer > 0 && support_regions.size() > 1)
				layer--;

		}
		else // place successfully
			// if (not highest layer && multilayer && (modelnum >= 2 || take up too much area))
		{	if (layer > 0 && support_regions.size() > 1
				&& (support_region->ModelNum >= 2 || !support_region->IsSpaceEnough()))
			{
				layer--;
			}
		}		
	}	
}

void FurnitureModel::ClearDecorationLayout()
{
	decoration_models.clear();
	for (size_t i = 0; i < this->support_regions.size(); i++)
	{
		support_regions[i]->Clear();
	}
}

void FurnitureModel::AdaptDecorationLocationType(DecorationModel *model) const
{
	for (size_t i = 0; i < model->LocationTypes.size(); i++)
	{
		switch (FurnitureFrontDirection)
		{
		case Invalid:
			break;
		case XPos:
			if (model->LocationTypes[i] == Left) model->LocationTypes[i] = Front;
			else if (model->LocationTypes[i] == Right) model->LocationTypes[i] = Back;
			else if (model->LocationTypes[i] == Front) model->LocationTypes[i] = Right;
			else if (model->LocationTypes[i] == Back) model->LocationTypes[i] = Left;
			else model->LocationTypes[i] = model->LocationTypes[i]; // keep unchanged
			break;
		case XNeg:
			if (model->LocationTypes[i] == Left) model->LocationTypes[i] = Back;
			else if (model->LocationTypes[i] == Right) model->LocationTypes[i] = Front;
			else if (model->LocationTypes[i] == Front) model->LocationTypes[i] = Left;
			else if (model->LocationTypes[i] == Back) model->LocationTypes[i] = Right;
			else model->LocationTypes[i] = model->LocationTypes[i]; // keep unchanged
			break;
		case ZPos:
			break;
		case ZNeg:
			if (model->LocationTypes[i] == Left) model->LocationTypes[i] = Right;
			else if (model->LocationTypes[i] == Right) model->LocationTypes[i] = Left;
			else if (model->LocationTypes[i] == Front) model->LocationTypes[i] = Back;
			else if (model->LocationTypes[i] == Back) model->LocationTypes[i] = Front;
			else model->LocationTypes[i] = model->LocationTypes[i]; // keep unchanged
			break;
		default:
			break;
		}
	}
}

bool FurnitureModel::IsDecorationAdded(QString& decorationtype)
{
	for (size_t i = 0; i < decoration_models.size(); i++)
	{
		if (QString::compare(decoration_models[i]->Type,decorationtype) == 0)
		{
			return true;
		}
	}
	return false;
}

// 得到相对于旋转之前的模型的坐标
void FurnitureModel::AdaptTranslateAccord2FrontDirection(float& tx, float& tz)
{
	float tmpx = tx;
	float tmpz = tz;
	switch (FurnitureFrontDirection)
	{
	case Invalid:
		break;
	case XPos:		
		break;
	case XNeg:
		tx = -tmpx;
		tz = -tmpz;
		break;
	case ZPos:
		tx = tmpz;
		tz = -tmpx;
		break;
	case ZNeg:
		tx = tmpz;
		tz = tmpx;
		break;
	default:
		break;
	}
}

void FurnitureModel::ToggleTextureOn()
{
	IsShowTexture = !IsShowTexture;
	updateTextureState();
}

QVector3D& FurnitureModel::getTranslate(float x, float y, float z)
{
	float tx = 0, ty = 0, tz = 0;
	switch (FurnitureFrontDirection)
	{
	case Invalid:
		break;
	case XPos:
		
		break;
	case XNeg:
		break;
	case ZPos:
		break;
	case ZNeg:
		break;
	default:
		break;
	}
	return QVector3D(tx, ty, tz);
}
