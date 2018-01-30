#ifndef VERTEX_H
#define VERTEX_H

#include <QOpenGLFunctions>
#include <QVector3D>
#include <QVector2D>


class Vertex
{
public:
	// Constructors
	Q_DECL_CONSTEXPR Vertex();
	Q_DECL_CONSTEXPR Vertex(const QVector3D &position);
	Q_DECL_CONSTEXPR Vertex(const QVector3D &position, const QVector3D &color);
	Q_DECL_CONSTEXPR Vertex(const QVector3D &position, const QVector2D &texture);
	Q_DECL_CONSTEXPR Vertex(const QVector3D &position, const QVector3D &normal, const QVector2D &texture);
	// Accessors / Mutators
	Q_DECL_CONSTEXPR const QVector3D& position() const;
	Q_DECL_CONSTEXPR const QVector3D& color() const;
	Q_DECL_CONSTEXPR const QVector3D& normal() const;
	Q_DECL_CONSTEXPR const QVector2D& texture() const;

	void setPosition(const QVector3D& position);
	void setColor(const QVector3D& color);
	void setNormal(const QVector3D& normal);
	void setTexture(const QVector2D& texture);

	// OpenGL Helpers
	static const int PositionTupleSize = 3;
	static const int ColorTupleSize = 3;
	static const int NormalTupleSize = 3;
	static const int TextureTupleSize = 2;
	static Q_DECL_CONSTEXPR int positionOffset();
	static Q_DECL_CONSTEXPR int colorOffset();
	static Q_DECL_CONSTEXPR int normalOffset();
	static Q_DECL_CONSTEXPR int textureOffset();
	static Q_DECL_CONSTEXPR int stride();

private:
	QVector3D m_position;	
	QVector3D m_normal;	
	QVector2D m_texCoords;
	QVector3D m_color;
};

/*******************************************************************************
 * Inline Implementation
 ******************************************************************************/
 
// Note: Q_MOVABLE_TYPE means it can be memcpy'd.
Q_DECLARE_TYPEINFO(Vertex, Q_MOVABLE_TYPE);
 
// Constructors
Q_DECL_CONSTEXPR inline Vertex::Vertex() {}
Q_DECL_CONSTEXPR inline Vertex::Vertex(const QVector3D &position) : m_position(position) {}
Q_DECL_CONSTEXPR inline Vertex::Vertex(const QVector3D &position, const QVector3D &color) : m_position(position), m_color(color) {}
Q_DECL_CONSTEXPR inline Vertex::Vertex(const QVector3D& position, const QVector2D& texture) : m_position(position), m_texCoords(texture){}
Q_DECL_CONSTEXPR inline Vertex::Vertex(const QVector3D &position, const QVector3D &normal, const QVector2D &texture)
	: m_position(position), m_normal(normal),m_texCoords(texture) {}
 
// Accessors / Mutators
Q_DECL_CONSTEXPR inline const QVector3D& Vertex::position() const { return m_position; }
Q_DECL_CONSTEXPR inline const QVector3D& Vertex::color() const { return m_color; }
Q_DECL_CONSTEXPR inline const QVector3D& Vertex::normal() const { return m_normal; }
Q_DECL_CONSTEXPR inline const QVector2D& Vertex::texture() const { return m_texCoords; }


void inline Vertex::setPosition(const QVector3D& position) { m_position = position; }
void inline Vertex::setColor(const QVector3D& color) { m_color = color; }
void inline Vertex::setNormal(const QVector3D& normal) { m_normal = normal; }
inline void Vertex::setTexture(const QVector2D& texture) { m_texCoords = texture; }

 
// OpenGL Helpers
Q_DECL_CONSTEXPR inline int Vertex::positionOffset() { return offsetof(Vertex, m_position); }
Q_DECL_CONSTEXPR inline int Vertex::colorOffset() { return offsetof(Vertex, m_color); }
Q_DECL_CONSTEXPR inline int Vertex::normalOffset() { return offsetof(Vertex, m_normal); }
Q_DECL_CONSTEXPR inline int Vertex::textureOffset() { return offsetof(Vertex, m_texCoords); }


Q_DECL_CONSTEXPR inline int Vertex::stride() { return sizeof(Vertex); }


#endif


