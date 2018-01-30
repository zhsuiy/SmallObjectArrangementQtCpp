#include "DisplayGLWidget.h"
#include "Vertex.h"
#include <QKeyEvent>

#include <QDebug>
#include <iostream>

static float ScreenWidth = 1400;
static float ScreenHeight = 800;
// Create a colored triangle
static const Vertex sg_vertexes[] = {
	// Positions           // Normals           // Texture Coords
	Vertex(QVector3D(-0.5f, -0.5f, -0.5f), QVector3D(0.0f,  0.0f, -1.0f) ,QVector2D(0.0f, 0.0f)),
	Vertex(QVector3D(0.5f, -0.5f, -0.5f), QVector3D(0.0f,  0.0f, -1.0f) ,QVector2D(1.0f, 0.0f)),
	Vertex(QVector3D(0.5f,  0.5f, -0.5f), QVector3D(0.0f,  0.0f, -1.0f) ,QVector2D(1.0f, 1.0f)),
	Vertex(QVector3D(0.5f,  0.5f, -0.5f), QVector3D(0.0f,  0.0f, -1.0f) ,QVector2D(1.0f, 1.0f)),
	Vertex(QVector3D(-0.5f,  0.5f, -0.5f), QVector3D(0.0f,  0.0f, -1.0f) ,QVector2D(0.0f, 1.0f)),
	Vertex(QVector3D(-0.5f, -0.5f, -0.5f), QVector3D(0.0f,  0.0f, -1.0f) ,QVector2D(0.0f, 0.0f)),

	Vertex(QVector3D(-0.5f, -0.5f,  0.5f), QVector3D(0.0f,  0.0f, 1.0f) ,QVector2D(0.0f, 0.0f)),
	Vertex(QVector3D(0.5f, -0.5f,  0.5f), QVector3D(0.0f,  0.0f, 1.0f) ,QVector2D(1.0f, 0.0f)),
	Vertex(QVector3D(0.5f,  0.5f,  0.5f), QVector3D(0.0f,  0.0f, 1.0f) ,QVector2D(1.0f, 1.0f)),
	Vertex(QVector3D(0.5f,  0.5f,  0.5f), QVector3D(0.0f,  0.0f, 1.0f) ,QVector2D(1.0f, 1.0f)),
	Vertex(QVector3D(-0.5f,  0.5f,  0.5f), QVector3D(0.0f,  0.0f, 1.0f) ,QVector2D(0.0f, 1.0f)),
	Vertex(QVector3D(-0.5f, -0.5f,  0.5f), QVector3D(0.0f,  0.0f, 1.0f) ,QVector2D(0.0f, 0.0f)),

	Vertex(QVector3D(-0.5f,  0.5f,  0.5f), QVector3D(-1.0f,  0.0f,  0.0f) ,QVector2D(1.0f, 0.0f)),
	Vertex(QVector3D(-0.5f,  0.5f, -0.5f), QVector3D(-1.0f,  0.0f,  0.0f) ,QVector2D(1.0f, 1.0f)),
	Vertex(QVector3D(-0.5f, -0.5f, -0.5f), QVector3D(-1.0f,  0.0f,  0.0f) ,QVector2D(0.0f, 1.0f)),
	Vertex(QVector3D(-0.5f, -0.5f, -0.5f), QVector3D(-1.0f,  0.0f,  0.0f) ,QVector2D(0.0f, 1.0f)),
	Vertex(QVector3D(-0.5f, -0.5f,  0.5f), QVector3D(-1.0f,  0.0f,  0.0f) ,QVector2D(0.0f, 0.0f)),
	Vertex(QVector3D(-0.5f,  0.5f,  0.5f), QVector3D(-1.0f,  0.0f,  0.0f) ,QVector2D(1.0f, 0.0f)),

	Vertex(QVector3D(0.5f,  0.5f,  0.5f), QVector3D(1.0f,  0.0f,  0.0f) ,QVector2D(1.0f, 0.0f)),
	Vertex(QVector3D(0.5f,  0.5f, -0.5f), QVector3D(1.0f,  0.0f,  0.0f) ,QVector2D(1.0f, 1.0f)),
	Vertex(QVector3D(0.5f, -0.5f, -0.5f), QVector3D(1.0f,  0.0f,  0.0f) ,QVector2D(0.0f, 1.0f)),
	Vertex(QVector3D(0.5f, -0.5f, -0.5f), QVector3D(1.0f,  0.0f,  0.0f) ,QVector2D(0.0f, 1.0f)),
	Vertex(QVector3D(0.5f, -0.5f,  0.5f), QVector3D(1.0f,  0.0f,  0.0f) ,QVector2D(0.0f, 0.0f)),
	Vertex(QVector3D(0.5f,  0.5f,  0.5f), QVector3D(1.0f,  0.0f,  0.0f) ,QVector2D(1.0f, 0.0f)),

	Vertex(QVector3D(-0.5f, -0.5f, -0.5f), QVector3D(0.0f, -1.0f,  0.0f) ,QVector2D(0.0f, 1.0f)),
	Vertex(QVector3D(0.5f, -0.5f, -0.5f), QVector3D(0.0f, -1.0f,  0.0f) ,QVector2D(1.0f, 1.0f)),
	Vertex(QVector3D(0.5f, -0.5f,  0.5f), QVector3D(0.0f, -1.0f,  0.0f) ,QVector2D(1.0f, 0.0f)),
	Vertex(QVector3D(0.5f, -0.5f,  0.5f), QVector3D(0.0f, -1.0f,  0.0f) ,QVector2D(1.0f, 0.0f)),
	Vertex(QVector3D(-0.5f, -0.5f,  0.5f), QVector3D(0.0f, -1.0f,  0.0f) ,QVector2D(0.0f, 0.0f)),
	Vertex(QVector3D(-0.5f, -0.5f, -0.5f), QVector3D(0.0f, -1.0f,  0.0f) ,QVector2D(0.0f, 1.0f)),

	Vertex(QVector3D(-0.5f,  0.5f, -0.5f), QVector3D(0.0f,  1.0f,  0.0f) ,QVector2D(0.0f, 1.0f)),
	Vertex(QVector3D(0.5f,  0.5f, -0.5f), QVector3D(0.0f,  1.0f,  0.0f) ,QVector2D(1.0f, 1.0f)),
	Vertex(QVector3D(0.5f,  0.5f,  0.5f), QVector3D(0.0f,  1.0f,  0.0f) ,QVector2D(1.0f, 0.0f)),
	Vertex(QVector3D(0.5f,  0.5f,  0.5f), QVector3D(0.0f,  1.0f,  0.0f) ,QVector2D(1.0f, 0.0f)),
	Vertex(QVector3D(-0.5f,  0.5f,  0.5f), QVector3D(0.0f,  1.0f,  0.0f) ,QVector2D(0.0f, 0.0f)),
	Vertex(QVector3D(-0.5f,  0.5f, -0.5f), QVector3D(0.0f,  1.0f,  0.0f) ,QVector2D(0.0f, 1.0f))
};

////static const GLfloat vertices[] = {
////	// Positions // Colors // Texture Coords
////	0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // Top Right
////	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Bottom Right
////	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom Left
////	-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f // Top Left
////};

//static const GLfloat vertices[] = {
//	// Positions // Colors // Texture Coords
//	0.5f, 0.5f, 0.0f, 1.0f, 1.0f, // Top Right
//	0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Bottom Right
//	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom Left
//	-0.5f, 0.5f, 0.0f, 0.0f, 1.0f // Top Left
//};
//static const GLfloat vertices[] = {
//	-1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
//	1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
//	1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
//	1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
//	-1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
//	-1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
//
//	-1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
//	1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
//	1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
//	1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
//	-1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
//	-1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
//
//	-1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
//	-1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
//	-1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
//	-1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
//	-1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
//	-1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
//
//	1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
//	1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
//	1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
//	1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
//	1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
//	1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
//
//	-1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
//	1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
//	1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
//	1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
//	-1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
//	-1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
//
//	-1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
//	1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
//	1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
//	1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
//	-1.0f,  1.0f,  1.0f,  0.0f, 0.0f,
//	-1.0f,  1.0f, -1.0f,  0.0f, 1.0f
//};

static const GLfloat vertices[] =
{
	// Positions           // Normals           // Texture Coords
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

// Positions all containers
static const QVector3D cubePositions[] = {
	QVector3D(0.0f,  0.0f,  0.0f),
	QVector3D(2.0f,  5.0f, -15.0f),
	QVector3D(-1.5f, -2.2f, -2.5f),
	QVector3D(-3.8f, -2.0f, -12.3f),
	QVector3D(2.4f, -0.4f, -3.5f),
	QVector3D(-1.7f,  3.0f, -7.5f),
	QVector3D(1.3f, -2.0f, -2.5f),
	QVector3D(1.5f,  2.0f, -2.5f),
	QVector3D(1.5f,  0.2f, -1.5f),
	QVector3D(-1.3f,  1.0f, -1.5f)
};

static const QVector3D pointLightPositions[] = {
	QVector3D(0.7f, 0.2f, 2.0f),
	QVector3D(2.3f, -3.3f, -4.0f),
	QVector3D(-4.0f, 2.0f, -12.0f),
	QVector3D(0.0f, 0.0f, -3.0f)
};

static const GLfloat vertices1[] = 
{
-0.5f, 0.5f, 0.0f, // Top Left
-0.5f,0.0f,	0.0f,
0.0f, 0.5f, 0.0f
};


static const GLuint indices[] = { // Note that we start from 0!
0,1,2, // First Triangle
3,4,5,
6,7,8,
9,10,11,
12,13,14,
15,16,17,
18,19,20,
21,22,23,
24,25,26,
27,28,29,
30,31,32,
33,34,35
//1, 2, 3 // Second Triangle
};




DisplayGLWidget::DisplayGLWidget(QWidget* parent)
	:QGLWidget(parent)
	//: QGLWidget(QGLFormat(QGL::SampleBuffers),parent)
	, m_vbo(QOpenGLBuffer::VertexBuffer)
	, m_ebo(QOpenGLBuffer::IndexBuffer)
	
{


	//matrix.translate(0.5, -0.5, 0);
	//matrix.rotate(rotation);
	//modelMatrix.translate(0, 0, -5);
	//modelMatrix.rotate(-55, 1, 0, 0);
	modelMatrix.setToIdentity();
	/*QMatrix4x4 viewMatrix;
	viewMatrix.translate(0, 0, -3);*/
	
	viewMatrix.translate(0.0, 0.0, -5.0);
	//viewMatrix.rotate(rotation);

	//model2camera = viewMatrix*modelMatrix;
	
	projection.perspective(45, ScreenWidth / ScreenHeight, 0.1f, 100.0f);

	//zoom = 1.0f;
	camera = new Camera(QVector3D(0.0, 0.0, 3.0));
	
	
	//timer.start();
	
}

DisplayGLWidget::~DisplayGLWidget()
{
	makeCurrent();
	teardownGL();
}

void DisplayGLWidget::teardownGL()
{
	 // Actually destroy our OpenGL information
	m_vbo.destroy();
	m_ebo.destroy();
	m_vao.destroy();
    delete m_program;
}

void DisplayGLWidget::keyPressEvent(QKeyEvent* event)
{	
	switch (event->key())
	{
	case Qt::Key_Up:
		mix = (mix + 0.1) > 1 ? 1 : mix + 0.1;
		std::cout << "mix = " << mix << endl;		
		break;
	case Qt::Key_Down:
		mix = (mix - 0.1) < 0 ? 0 : mix - 0.1;
		std::cout << "mix = " << mix << endl;		
		break;
	case Qt::Key_W:
		camera->ProcessKeyboard(FORWARD);
		break;
	case Qt::Key_S:
		camera->ProcessKeyboard(BACKWARD);
		break;
	case Qt::Key_A:
		camera->ProcessKeyboard(LEFT);
		break;
	case Qt::Key_D:
		camera->ProcessKeyboard(RIGHT);
		break;
	case Qt::Key_R:
		camera->Reset();
		break;
	default:
		break;
	}
	update();
}

void DisplayGLWidget::initializeGL()
{
	//glShadeModel(GL_SMOOTH);		// 启用阴影光滑
	//glClearColor(0.2,0.2,0.2,0);	// 设置清除屏幕的颜色
	//glClearDepth(1.0);				// 设置深度缓存
	 //glEnable(GL_DEPTH_TEST);		// 深度测试
	//glDepthFunc(GL_LEQUAL);			// 启用深度测试
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);	// 进行最好的透视修正，可能会影响性能
	 initializeOpenGLFunctions();
	 // Enable depth buffer
	 glEnable(GL_DEPTH_TEST);
	 // Enable back face culling
	 //glEnable(GL_CULL_FACE);

	 printVersionInformation();
	 glClearColor(0.3,0.3,0.3,0);	

	 // Application-specific initialization

	{
    // Create Shader (Do not release until VAO is created)
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/mesh.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/mesh.frag");
    m_program->link();
    m_program->bind();
 
	// Create Vertex Array Object
    m_vao.create();
    m_vao.bind();


	QVector<Vertex> myvertices;
	for (size_t i = 0; i < 36; i++)
	{
		myvertices.push_back(sg_vertexes[i]);
	}
    // Create Buffer (Do not release until VAO is created)
    m_vbo.create();
    m_vbo.bind();
    m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    //m_vbo.allocate(vertices, sizeof(vertices));
	//m_vbo.allocate(sg_vertexes, sizeof(sg_vertexes));
	m_vbo.allocate(myvertices.constData(), myvertices.size()*sizeof(Vertex));

	m_ebo.create();	
	m_ebo.bind();	
	m_ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_ebo.allocate(indices,sizeof(indices));
 
   
    m_program->enableAttributeArray(0);    
	m_program->enableAttributeArray(1);
	m_program->enableAttributeArray(2);

	

	//m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat));      // 3表示的是这一个属性里面有几个分量
	//m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
	//m_program->setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(GLfloat), 2, 8 * sizeof(GLfloat));

	m_program->setAttributeBuffer(0, GL_FLOAT, Vertex::positionOffset(), Vertex::PositionTupleSize, Vertex::stride());
	m_program->setAttributeBuffer(1, GL_FLOAT, Vertex::normalOffset(), Vertex::NormalTupleSize, Vertex::stride());
	m_program->setAttributeBuffer(2, GL_FLOAT, Vertex::textureOffset(), Vertex::TextureTupleSize, Vertex::stride());


	initTextures();
	
    // Release (unbind) all	
	m_vbo.release();
	m_vao.release();    
	//m_ebo.release();
	m_program->release();	
  }	

	initLights();

}

void DisplayGLWidget::paintGL()
{
	//makeCurrent();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);	// 清除屏幕和深度缓存
	//glLoadIdentity();									// 重置当前模型的观察矩阵
	 // Render using our shader
	
	

	m_program->bind();
	{		
		
		//m_program->setUniformValue("ourTexture", 0);
		texture->bind();	
		m_program->setUniformValue("material.diffuse", 0);
		m_program->setUniformValue("material.specular", 1);
		m_program->setUniformValue("ourTexture1", 1);
		//m_program->setUniformValue("material.diffuse", 1);
		texture1->bind(1);
		m_program->setUniformValue("lambda", mix);
		texture2->bind(2);
		m_program->setUniformValue("material.emission", 2);
		

		//m_program->enableAttributeArray("ourTexture1");

		

		
		viewMatrix = camera->GetViewMatrix();
		
		//viewMatrix.translate(0.0, 0.0, -5.0);
		//viewMatrix.rotate(rotation);
		//matrix.scale(0.5);
		projection.setToIdentity();
		projection.perspective(camera->Zoom, (float)ScreenWidth / (float)ScreenHeight, 0.1f, 100.0f);
				

		//m_program->setUniformValue("transform", projection*viewMatrix*matrix);
		m_program->setUniformValue("viewMatrix", viewMatrix);
		m_program->setUniformValue("projection", projection);
		m_program->setUniformValue("viewPos", camera->Position);

			

		// lighting
		m_program->setUniformValue("dirLight.direction", -0.2f, -1.0f, -0.3f);
		m_program->setUniformValue("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		m_program->setUniformValue("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		m_program->setUniformValue("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// Point light 1
		m_program->setUniformValue("pointLights[0].position", pointLightPositions[0]);
		m_program->setUniformValue("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		m_program->setUniformValue("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		m_program->setUniformValue("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		m_program->setUniformValue("pointLights[0].constant", 1.0f);
		m_program->setUniformValue("pointLights[0].linear", 0.09f);
		m_program->setUniformValue("pointLights[0].quadratic", 0.032f);
		// Point light 2
		m_program->setUniformValue("pointLights[1].position", pointLightPositions[1]);
		m_program->setUniformValue("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		m_program->setUniformValue("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		m_program->setUniformValue("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		m_program->setUniformValue("pointLights[1].constant", 1.0f);
		m_program->setUniformValue("pointLights[1].linear", 0.09f);
		m_program->setUniformValue("pointLights[1].quadratic", 0.032f);
		// Point light 3
		m_program->setUniformValue("pointLights[2].position", pointLightPositions[2]);
		m_program->setUniformValue("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		m_program->setUniformValue("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		m_program->setUniformValue("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		m_program->setUniformValue("pointLights[2].constant", 1.0f);
		m_program->setUniformValue("pointLights[2].linear", 0.09f);
		m_program->setUniformValue("pointLights[2].quadratic", 0.032f);
		// Point light 4
		m_program->setUniformValue("pointLights[3].position", pointLightPositions[3]);
		m_program->setUniformValue("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		m_program->setUniformValue("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		m_program->setUniformValue("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		m_program->setUniformValue("pointLights[3].constant", 1.0f);
		m_program->setUniformValue("pointLights[3].linear", 0.09f);
		m_program->setUniformValue("pointLights[3].quadratic", 0.032f);

		
		
	

		//m_program->setUniformValue("material.ambient", QVector3D(0.25f, 0.20725f, 0.20725f));
		//m_program->setUniformValue("material.diffuse", QVector3D(1.0f, 0.829f, 0.829f));
		
		
		m_program->setUniformValue("material.shininess", 64.0f);




		m_vao.bind();
		//for (int i = 0; i < 10; i++)
		//{
			modelMatrix.setToIdentity();
		//	modelMatrix.translate(cubePositions[i]);
		//	float angle = 20.0f * i;
		//	modelMatrix.rotate(angle, QVector3D(1.0f, 0.3f, 0.5f));
			m_program->setUniformValue("modelMatrix", modelMatrix);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
			//glDrawArrays(GL_TRIANGLES, 0, sizeof(sg_vertexes) / sizeof(sg_vertexes[0]));
			//glDrawArrays(GL_TRIANGLES, 0, 36);
	//	}		


		m_vao.release();
		
	}
	m_program->release();

	paintLight();

	/*m_program1->bind();
	{		
		m_vao1.bind();
		glDrawArrays(GL_TRIANGLES,0,3);
		m_vao1.release();
	}
	m_program1->release();*/
}


void DisplayGLWidget::resizeGL(int width, int height)
{
	
	
	glViewport(0,0,(GLint)width,(GLint)height);	// 重置当前的视口
	//glMatrixMode(GL_PROJECTION);				// 选择投影矩阵
	//glLoadIdentity();							// 重置投影矩阵
	////gluPerspective(45.0,(GLfloat)width/(GLfloat)height,0.1,100.0); // 建立透视投影矩阵
	//glMatrixMode(GL_MODELVIEW); // 选择模型观察矩阵
	//glLoadIdentity();			// 重置模型观察矩阵

	projection.setToIdentity();

	// Set perspective projection
	projection.perspective(45, qreal(width)/qreal(height?height:1), 0.1f, 100.0f);
	
	
}

void DisplayGLWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mouseButton = Qt::LeftButton;
		mouseCurPos = QVector2D(event->localPos());
		mouseLastPos = QVector2D(event->localPos());
		//std::cout << "local pos" << mouseCurrentPosition.x << "," << mouseCurrentPosition.y() << endl;
	}
	else if (event->button() == Qt::RightButton)
	{
		mouseButton = Qt::RightButton;
		mouseCurPos = QVector2D(event->localPos());
		mouseLastPos = QVector2D(event->localPos());
	}
	
	
}
QVector3D DisplayGLWidget::getArcBallVector(int x, int y)
{
	QVector3D P = QVector3D(1.0*x / ScreenWidth * 2 - 1.0,
		1.0*y / ScreenHeight * 2 - 1.0,
		0);
	P.setY(-P.y());
	float OP_squared = P.x() * P.x() + P.y() * P.y();
	if (OP_squared <= 1 * 1)
		P.setZ(sqrt(1 * 1 - OP_squared));  // Pythagore
	else
		P.normalize(); // nearest point
	return P;
	
}


void DisplayGLWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (mouseButton == Qt::LeftButton)
	{
		mouseCurPos.setX(event->localPos().x());
		mouseCurPos.setY(event->localPos().y());	

		if (mouseCurPos.x() != mouseLastPos.x() || mouseCurPos.y() != mouseLastPos.y()) 
		{
			QVector3D va = getArcBallVector(mouseLastPos.x(), mouseLastPos.y());
			QVector3D vb = getArcBallVector(mouseCurPos.x(), mouseCurPos.y());
			float angle = acos(std::min(1.0f, QVector3D::dotProduct(va, vb)))*100;
			QVector3D axis_in_camera_coord = QVector3D::crossProduct(va,vb);
			
			//viewMatrix.setToIdentity();
			//viewMatrix.translate(0.0, 0.0, -5.0);
			//model2camera.rotate(angle, axis_in_camera_coord);
			rotation = QQuaternion::fromAxisAndAngle(axis_in_camera_coord, angle) * rotation;
			camera->ProcessMouseMovement(mouseCurPos.x() - mouseLastPos.x(),mouseLastPos.y() - mouseCurPos.y());
			mouseLastPos.setX(mouseCurPos.x());
			mouseLastPos.setY(mouseCurPos.y());
			update();						
		}
	}

	else if (mouseButton == Qt::RightButton)
	{
		mouseCurPos.setX(event->localPos().x());
		mouseCurPos.setY(event->localPos().y());
		camera->ProcessRightMouseMovement(mouseCurPos.x() - mouseLastPos.x(), mouseLastPos.y() - mouseCurPos.y());
		mouseLastPos.setX(mouseCurPos.x());
		mouseLastPos.setY(mouseCurPos.y());
		update();

	}
	//// Mouse release position - mouse press position
	//QVector2D diff = QVector2D(event->localPos()) - mousePressPosition;

	//// Rotation axis is perpendicular to the mouse position difference
	//// vector
	//QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

	//// Accelerate angular speed relative to the length of the mouse sweep
	//qreal acc = diff.length() / 100.0;

	//// Calculate new rotation axis as weighted sum
	//rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();

	//// Increase angular speed
	//angularSpeed += acc;

	//mousePressPosition.setX(event->localPos().x());
	//mousePressPosition.setY(event->localPos().y());

	//rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;

	//update();
	
}

void DisplayGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
	mouseButton = Qt::NoButton;

	
	
}

void DisplayGLWidget::wheelEvent(QWheelEvent* event)
{
	int diff = event->delta() - scrollDelta;

	camera->ProcessMouseScroll(event->delta()/120);
	
	scrollDelta = event->delta();
	
	update();
}

void DisplayGLWidget::printVersionInformation()
{
	QString glType;
	QString glVersion;
	QString glProfile;
 
	// Get Version Information
	//glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
	glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
 
	// Get Profile Information
#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
  switch (format().profile())
  {
    CASE(NoProfile);
    CASE(CoreProfile);
    CASE(CompatibilityProfile);
  }
#undef CASE
 
  // qPrintable() will print our QString w/o quotes around it.
  //qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
  qDebug() << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
}

void DisplayGLWidget::initTextures()
{
	texture = new QOpenGLTexture(QImage("./resources/texture/woodmetal.png").mirrored());		
	texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	texture->setMagnificationFilter(QOpenGLTexture::Linear);
	texture->setWrapMode(QOpenGLTexture::ClampToBorder);
	

	texture1 = new QOpenGLTexture(QImage("./resources/texture/woodmetal_specular.png").mirrored());
	texture1->setMinificationFilter(QOpenGLTexture::Nearest);
	texture1->setMagnificationFilter(QOpenGLTexture::Nearest);
	texture1->setWrapMode(QOpenGLTexture::Repeat);

	texture2 = new QOpenGLTexture(QImage("./resources/texture/matrix.jpg").mirrored());
	texture2->setMinificationFilter(QOpenGLTexture::Nearest);
	texture2->setMagnificationFilter(QOpenGLTexture::Nearest);
	texture2->setWrapMode(QOpenGLTexture::Repeat);


}

void DisplayGLWidget::initLights()
{
	
	// Create Shader (Do not release until VAO is created)
	light_program = new QOpenGLShaderProgram();
	light_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/light.vert");
	light_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/light.frag");
	light_program->link();
	light_program->bind();

	// Create Vertex Array Object
	light_vao.create();
	light_vao.bind();

	QVector<Vertex> myvertices;
	for (size_t i = 0; i < 36; i++)
	{
		myvertices.push_back(sg_vertexes[i]);
	}

	// Create Buffer (Do not release until VAO is created)
	m_vbo.create();
	m_vbo.bind();
	m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	//m_vbo.allocate(vertices, sizeof(vertices));
	m_vbo.allocate(myvertices.constData(), 36*sizeof(Vertex));

	m_ebo.create();
	m_ebo.bind();
	m_ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_ebo.allocate(indices, sizeof(indices));


	light_program->enableAttributeArray(0);
	
	//m_program->enableAttributeArray(2);
	//   m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3 , 8*sizeof(GLfloat));      // 3表示的是这一个属性里面有几个分量
	//m_program->setAttributeBuffer(1, GL_FLOAT, 3*sizeof(GLfloat), 3,  8*sizeof(GLfloat));
	//m_program->setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(GLfloat), 2, 8 * sizeof(GLfloat));
	//

	//light_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat));      // 3表示的是这一个属性里面有几个分量
	light_program->setAttributeBuffer(0, GL_FLOAT, Vertex::positionOffset(), Vertex::PositionTupleSize, Vertex::stride());
			

	// Release (unbind) all	
	m_vbo.release();
	light_vao.release();
	//m_ebo.release();
	light_program->release();
	

}

void DisplayGLWidget::paintLight()
{
	light_program->bind();
	{	

		
		viewMatrix = camera->GetViewMatrix();
		//viewMatrix.translate(0.0, 0.0, -5.0);
		//viewMatrix.rotate(rotation);
		//matrix.scale(0.5);
		projection.setToIdentity();
		projection.perspective(camera->Zoom, (float)ScreenWidth / (float)ScreenHeight, 0.1f, 100.0f);
		

		//m_program->setUniformValue("transform", projection*viewMatrix*matrix);
		light_program->setUniformValue("viewMatrix", viewMatrix);
		
		light_program->setUniformValue("projection", projection);
		light_vao.bind();

		for (int i = 0; i < 4; i++)
		{
			QMatrix4x4 model;
			model.translate(pointLightPositions[i]);
			model.scale(0.2f);
			light_program->setUniformValue("modelMatrix", model);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
			//glDrawArrays(GL_TRIANGLES, 0, sizeof(sg_vertexes) / sizeof(sg_vertexes[0]));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		


		
		




		light_vao.release();
	}
	light_program->release();
}
