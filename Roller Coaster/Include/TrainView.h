#ifndef TRAINVIEW_H  
#define TRAINVIEW_H  
#include <QGLWidget> 
#include <QtGui>  
#include <QtOpenGL>  
#include <GL/GLU.h>
#include <iostream>
#include <cmath>
#include <ctime>
using namespace std;
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib") 
#include "Utilities/ArcBallCam.H"
#include "Utilities/3DUtils.H"
#include "Track.H"
#include "Triangle.h"
#include "Square.h"
#include "ogldev_math_3d.h"

class AppMain;
class CTrack;

//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


class TrainView : public QGLWidget, protected QOpenGLFunctions_4_3_Core
{  
	Q_OBJECT  
public:  
	explicit TrainView(QWidget *parent = 0);  
	~TrainView();  

public:
	// overrides of important window things
	//virtual int handle(int);
	virtual void paintGL();

	// all of the actual drawing happens in this routine
	// it has to be encapsulated(封裝), since we draw differently if
	// we're drawing shadows (no colors, for example)
	void drawStuff(bool doingShadows=false );
	//draw train
	void drawTrain(float , bool doingShadows = false);
	void cauculateLength();
	// setup the projection - assuming that the projection stack has been
	// cleared for you
	void setProjection();

	// Reset the Arc ball control
	void resetArcball();

	// pick a point (for when the mouse goes down)
	void doPick(int mx, int my);

	void initializeGL();
	void initializeTexture();
	Vector4f  matrix4x4x4x1(Matrix4f, Vector4f);
	Pnt3f  matrix3x4x4x1(Pnt3f, Pnt3f, Pnt3f, Pnt3f, Vector4f);

public:
	ArcBallCam		arcball;			// keep an ArcBall for the UI
	int				selectedCube;  // simple - just remember which cube is selected

	CTrack*			m_pTrack;		// The track of the entire scene

	int camera;   // top, world, train
	int curve;    //linear, cubic, spline
	int track;    //line, track, road
	bool isrun;
	float t_time = 0;
	unsigned int DIVIDE_LINE = 150;
	float tracklength = 0;
	float lengthcount = 0; //計算是否要畫枕木，>5就畫，<5存起來，加上下一段
	float trainpos_t = 0;  //火車走到哪個長度
	int sleeper = 0;
	vector<float> trackpercent;
	vector<float> partLength;  //每段鐵軌的長度
	Triangle* triangle;
	Square* square;
	GLfloat ProjectionMatrex[16];
	GLfloat ModelViewMatrex[16];
	QVector<QOpenGLTexture*> Textures;
	typedef enum {
	spline_Linear = 0,
	spline_CardinalCubic = 1,
	spline_CubicB_Spline = 2
	} spline_t;
	typedef enum {
		track_line = 0,
		track_track = 1,
		track_road = 2
	}track_t;
	Matrix4f Cardinal = { -0.5,  1.5, -1.5,  0.5,
						 	 1, -2.5,    2, -0.5,
						  -0.5,    0,  0.5,    0,
						   	 0,    1,    0,    0 };
	Matrix4f B_Spline = { -1.0 / 6.0,     0.5,  -0.5,  1.0 / 6.0,
						     0.5,      -1,   0.5,      0,
						    -0.5,       0,   0.5,      0,
						   1.0 / 6.0,  4.0 / 6.0,  1.0 / 6.0,      0 };
};  
#endif // TRAINVIEW_H  