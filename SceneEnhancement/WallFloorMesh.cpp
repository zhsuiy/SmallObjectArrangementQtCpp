#include "WallFloorMesh.h"
#include "Global.h"

WallFloorMesh::WallFloorMesh(QVector3D& leftBottomBack, QVector3D& rightTopFront, WallFloorType type, Material* material):Mesh()
{
	float neg_x = leftBottomBack.x();
	float neg_y = leftBottomBack.y();
	float neg_z = leftBottomBack.z();
	float pos_x = rightTopFront.x();
	float pos_y = rightTopFront.y();
	float pos_z = rightTopFront.z();

	switch (type)
	{
	case Ceiling:
		Vertices.push_back(Vertex(QVector3D(neg_x, pos_y, pos_z), QVector3D(0, 1, 0), QVector2D(0, 0))); // top
		Vertices.push_back(Vertex(QVector3D(pos_x, pos_y, pos_z), QVector3D(0, 1, 0), QVector2D(1, 0)));
		Vertices.push_back(Vertex(QVector3D(pos_x, pos_y, neg_z), QVector3D(0, 1, 0), QVector2D(1, 1)));
		Vertices.push_back(Vertex(QVector3D(neg_x, pos_y, neg_z), QVector3D(0, 1, 0), QVector2D(0, 1)));
		break;
	case LeftWall:
		Vertices.push_back(Vertex(QVector3D(neg_x, neg_y, pos_z), QVector3D(1, 0, 0), QVector2D(0, 0))); // left
		Vertices.push_back(Vertex(QVector3D(neg_x, neg_y, neg_z), QVector3D(1, 0, 0), QVector2D(1, 0)));
		Vertices.push_back(Vertex(QVector3D(neg_x, pos_y, neg_z), QVector3D(1, 0, 0), QVector2D(1, 1)));
		Vertices.push_back(Vertex(QVector3D(neg_x, pos_y, pos_z), QVector3D(1, 0, 0), QVector2D(0, 1)));
		break;
	case BackWall:
		Vertices.push_back(Vertex(QVector3D(neg_x, neg_y, neg_z), QVector3D(0, 0, 1), QVector2D(0, 0))); // back
		Vertices.push_back(Vertex(QVector3D(pos_x, neg_y, neg_z), QVector3D(0, 0, 1), QVector2D(1, 0)));
		Vertices.push_back(Vertex(QVector3D(pos_x, pos_y, neg_z), QVector3D(0, 0, 1), QVector2D(1, 1)));
		Vertices.push_back(Vertex(QVector3D(neg_x, pos_y, neg_z), QVector3D(0, 0, 1), QVector2D(0, 1)));
		break;
	case RightWall:
		Vertices.push_back(Vertex(QVector3D(pos_x, neg_y, neg_z), QVector3D(-1, 0, 0), QVector2D(0, 0))); // right
		Vertices.push_back(Vertex(QVector3D(pos_x, neg_y, pos_z), QVector3D(-1, 0, 0), QVector2D(1, 0)));
		Vertices.push_back(Vertex(QVector3D(pos_x, pos_y, pos_z), QVector3D(-1, 0, 0), QVector2D(1, 1)));
		Vertices.push_back(Vertex(QVector3D(pos_x, pos_y, neg_z), QVector3D(-1, 0, 0), QVector2D(0, 1)));
		break;
	case Floor:
		Vertices.push_back(Vertex(QVector3D(neg_x, neg_y, pos_z), QVector3D(0, 1, 0), QVector2D(0, 0))); // bottom
		Vertices.push_back(Vertex(QVector3D(pos_x, neg_y, pos_z), QVector3D(0, 1, 0), QVector2D(1, 0)));
		Vertices.push_back(Vertex(QVector3D(pos_x, neg_y, neg_z), QVector3D(0, 1, 0), QVector2D(1, 1)));
		Vertices.push_back(Vertex(QVector3D(neg_x, neg_y, neg_z), QVector3D(0, 1, 0), QVector2D(0, 1)));
		break;
	default:
		break;
	}

	if (Indices.isEmpty())
	{
		GLuint arr_indices[] = {
			0,1,2, // First Triangle
			0,2,3			
		};
		for (size_t i = 0; i < 6; i++)
		{
			Indices.push_back(arr_indices[i]);
		}
	}

	MeshMaterial = material;

	this->updateNormals();
	this->setupRender();
}
