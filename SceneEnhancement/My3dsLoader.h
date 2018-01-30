#ifndef MESHLOADER_H
#define MESHLOADER_H

#include<iostream>
#include<fstream>
#include<vector>
#include<set>
#include<string>
#include "lib3ds.h"
#include "MyVector3D.h"

namespace DataStruct
{
	struct MyObject
	{
		std::string label;
		std::set<int> meshIDs;
	};

	class My3dsLoader
	{
		public:
			My3dsLoader();
			~My3dsLoader();
			bool load3ds(const char* fname);
			void My3dsLoader::saveTransformed3ds(const char* fname, const MyVector3D translate, const float alpha);
			void extractObjects(const char* fname, const char* outputDirectory);
			void flatObjects(const char* output);
			const std::vector<Lib3dsMaterial> getMaterialArray();
			const std::vector<MyVector3D> getVertexArray();
			const std::vector<int> getFaceArray();
			const std::vector<int> getFaceIDArray();
			const std::vector<int> getFaceMaterialArray();
			void setName(std::string name);
			std::string getName();
			void setOrientation(float x,float y,float z);
			MyVector3D getOrientation();

			const char* getRootName();
			const MyVector3D getBbMin();
			const MyVector3D getBbMax();
			const MyVector3D getCenter();
			void toObj(const char* fname);
			void toOff(const char* fname);

		private:
			void extract_mesh(Lib3dsMeshInstanceNode *node);
			void extract_nodes(Lib3dsNode *first_node);
			void saveObject(int index,const char* fname);

			Lib3dsFile *file;
			std::vector<Lib3dsMaterial> materialArray;
			std::vector<MyVector3D> vectexArray;
			std::vector<int> faceArray;
			std::vector<int> faceIDArray;
			std::vector<int> faceMaterialArray;
			std::vector<MyObject> objects;
			std::string name;
			MyVector3D orientation;
			float bbMin[3],bbMax[3];
			int vCount;
	}; 
}


#endif