#include "TrainView.h"  

TrainView::TrainView(QWidget *parent) :  
QGLWidget(parent)  
{  
	resetArcball();
	
}  
Vector4f  TrainView::matrix4x4x4x1(Matrix4f m, Vector4f v) {
	Vector4f vr;   //vector4 return
	vr.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
	vr.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
	vr.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
	vr.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
	return vr;
}
Pnt3f  TrainView::matrix3x4x4x1(Pnt3f a, Pnt3f b, Pnt3f c, Pnt3f d, Vector4f e) {
	Pnt3f vr;   //Pnt3f return
	vr.x = a.x * e.x + b.x * e.y + c.x * e.z + d.x * e.w;
	vr.y = a.y * e.x + b.y * e.y + c.y * e.z + d.y * e.w;
	vr.z = a.z * e.x + b.z * e.y + c.z * e.z + d.z * e.w;
	return vr;
}
TrainView::~TrainView()  
{}  
void TrainView::initializeGL()
{
	initializeOpenGLFunctions();
	//Create a triangle object
	triangle = new Triangle();
	//Initialize the triangle object
	triangle->Init();
	//Create a square object
	square = new Square();
	//Initialize the square object
	square->Init();
	//Initialize texture 
	initializeTexture();
	
	
}
void TrainView::initializeTexture()
{
	//Load and create a texture for square;'stexture
	QOpenGLTexture* texture = new QOpenGLTexture(QImage("./Textures/Tupi.bmp"));
	Textures.push_back(texture);
}
void TrainView:: resetArcball()
	//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

void TrainView::paintGL()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// Set up the view port
	glViewport(0,0,width(),height());
	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0,0,0.3f,0);
	
	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (this->camera == 1) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	} else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[]	= {0,1,1,0}; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[]	= {1, 0, 0, 0};
	GLfloat lightPosition3[]	= {0, -1, 0, 0};
	GLfloat yellowLight[]		= {0.5f, 0.5f, .1f, 1.0};
	GLfloat whiteLight[]		= {1.0f, 1.0f, 1.0f, 1.0};
	GLfloat blueLight[]			= {.1f,.1f,.3f,1.0};
	GLfloat grayLight[]			= {.3f, .3f, .3f, 1.0};

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);



	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	setupFloor();
	glDisable(GL_LIGHTING);
	drawFloor(200,10);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();
	trackpercent.resize(m_pTrack->points.size());
	partLength.resize(m_pTrack->points.size());
	cauculateLength();
	drawStuff();
	drawTrain(t_time,false);
	// this time drawing is for shadows (except for top view)
	if (this->camera != 1) {
		setupShadows();
		drawStuff(true);
		drawTrain(t_time, true);
		unsetupShadows();
	}

	//Get modelview matrix
 	glGetFloatv(GL_MODELVIEW_MATRIX,ModelViewMatrex);
	//Get projection matrix
 	glGetFloatv(GL_PROJECTION_MATRIX,ProjectionMatrex);

	//Call triangle's render function, pass ModelViewMatrex and ProjectionMatrex
 	triangle->Paint(ProjectionMatrex,ModelViewMatrex);
    
	//we manage textures by Trainview class, so we modify square's render function
	square->Begin();
		//Active Texture
		glActiveTexture(GL_TEXTURE0);
		//Bind square's texture
		Textures[0]->bind();
		//pass texture to shader
		square->shaderProgram->setUniformValue("Texture",0);
		//Call square's render function, pass ModelViewMatrex and ProjectionMatrex
		square->Paint(ProjectionMatrex,ModelViewMatrex);
	square->End();
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(width()) / static_cast<float>(height());

	// Check whether we use the world camp
	if (this->camera == 0){
		arcball.setProjection(false);
		update();
	// Or we use the top cam
	}else if (this->camera == 1) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		} 
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90,1,0,0);
		update();
	} 
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this,aspect);
#endif
		update();
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::cauculateLength() {
	spline_t type_spline = (spline_t)curve;  //將curve(0,1,2)傳給spline type
	track_t type_track = (track_t)track;
	float tracklength_t = 0;
	for (int i = 0; i < m_pTrack->points.size(); i++) {
		float partlen = 0;
		/*
		Pnt3f pos_p1 = m_pTrack->points[i].pos;                //起點的pos
		Pnt3f pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;      //終點的pos
		Pnt3f pos_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;     
		Pnt3f pos_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].pos;      
		Pnt3f orient_p1 = m_pTrack->points[i].orient;
		Pnt3f orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
		Pnt3f orient_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;
		Pnt3f orient_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].orient;
		*/
		Pnt3f pos_p1 = m_pTrack->points[(i + m_pTrack->points.size()-1) % m_pTrack->points.size()].pos;                //起點的pos
		Pnt3f pos_p2 = m_pTrack->points[i % m_pTrack->points.size()].pos;      //終點的pos
		Pnt3f pos_p3 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;
		Pnt3f pos_p4 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;
		Pnt3f orient_p1 = m_pTrack->points[(i + m_pTrack->points.size()-1) % m_pTrack->points.size()].orient;
		Pnt3f orient_p2 = m_pTrack->points[i % m_pTrack->points.size()].orient;
		Pnt3f orient_p3 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
		Pnt3f orient_p4 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;
		Pnt3f qt, qt0, qt1, orient_t, cross_t;

		float percent = 1.0 / DIVIDE_LINE;
		float t = 0;
		Vector4f MT;
		switch (type_spline) {
		case spline_Linear:
			qt = (1 - t) * pos_p1 + t * pos_p2;
			break;
		case spline_CardinalCubic:
			MT = matrix4x4x4x1(Cardinal, Vector4f{ t*t*t, t*t, t, 1 });
			qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
			break;
		case spline_CubicB_Spline:
			MT = matrix4x4x4x1(B_Spline, Vector4f{ t*t*t, t*t, t, 1 });
			qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
			break;
		}
		for (int j = 0; j < DIVIDE_LINE; j++) {
			qt0 = qt;
			switch (type_spline) {
			case spline_Linear:
				orient_t = (1 - t) * orient_p1 + t * orient_p2;
				break;
			case spline_CardinalCubic:
				MT = matrix4x4x4x1(Cardinal, Vector4f{ t*t*t, t*t, t, 1 });
				orient_t = matrix3x4x4x1(orient_p1, orient_p2, orient_p3, orient_p4, MT);
				break;
			case spline_CubicB_Spline:
				MT = matrix4x4x4x1(B_Spline, Vector4f{ t*t*t, t*t, t, 1 });
				orient_t = matrix3x4x4x1(orient_p1, orient_p2, orient_p3, orient_p4, MT);
				break;
			}

			t += percent;
			switch (type_spline) {
			case spline_Linear:
				qt = (1 - t) * pos_p1 + t * pos_p2;
				break;
			case spline_CardinalCubic:
				MT = matrix4x4x4x1(Cardinal, Vector4f{ t*t*t, t*t, t, 1 });
				qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
				break;
			case spline_CubicB_Spline:
				MT = matrix4x4x4x1(B_Spline, Vector4f{ t*t*t, t*t, t, 1 });
				qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
				break;
			}
			qt1 = qt;
			tracklength_t += sqrt((qt1.x - qt0.x)*(qt1.x - qt0.x) + (qt1.y - qt0.y)*(qt1.y - qt0.y) + (qt1.z - qt0.z)*(qt1.z - qt0.z));
			partlen += sqrt((qt1.x - qt0.x)*(qt1.x - qt0.x) + (qt1.y - qt0.y)*(qt1.y - qt0.y) + (qt1.z - qt0.z)*(qt1.z - qt0.z));
		}
		partLength[i] = partlen;

		//trackpercent[i] = partlen / tracklength;
	}
	tracklength = tracklength_t;
	for (int i = 0; i < m_pTrack->points.size(); i++) {
		trackpercent[i] = partLength[i] / tracklength;
		//cout << i << " , " << trackpercent[i]<<endl;
	}
	
	//cout << tracklength << endl;
}
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (this->camera != 2) {
		for(size_t i = 0; i < this->m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if ( ((int) i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			this->m_pTrack->points[i].draw();
		}
		update();
	}

	// ===============================================draw the track  畫鐵軌=====================================================
	//cout << m_pTrack->points[0].pos.x << " , " << m_pTrack->points[0].pos.y << " , " << m_pTrack->points[0].pos.z<<endl;
	//cout << m_pTrack->points[0].orient.x << " , " << m_pTrack->points[0].orient.y << " , " << m_pTrack->points[0].orient.z << endl;
	spline_t type_spline = (spline_t) curve;  //將curve(0,1,2)傳給spline type
	track_t type_track = (track_t) track;
	float tracklength_t = 0;
	for (int i = 0; i < m_pTrack->points.size(); i++) {
		lengthcount = 0;
		sleeper = 0;
		/*
		Pnt3f pos_p1 = m_pTrack->points[i].pos;                //起點的pos
		Pnt3f pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;      //終點的pos
		Pnt3f pos_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;      
		Pnt3f pos_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].pos;     
		Pnt3f orient_p1 = m_pTrack->points[i].orient;
		Pnt3f orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
		Pnt3f orient_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;
		Pnt3f orient_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].orient;
		*/
		Pnt3f pos_p1 = m_pTrack->points[( i + m_pTrack->points.size()-1 ) % m_pTrack->points.size()].pos;                //起點的pos
		Pnt3f pos_p2 = m_pTrack->points[i % m_pTrack->points.size()].pos;      //終點的pos
		Pnt3f pos_p3 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;
		Pnt3f pos_p4 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;
		Pnt3f orient_p1 = m_pTrack->points[(i + m_pTrack->points.size()-1) % m_pTrack->points.size()].orient;
		Pnt3f orient_p2 = m_pTrack->points[i % m_pTrack->points.size()].orient;
		Pnt3f orient_p3 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
		Pnt3f orient_p4 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;
		Pnt3f qt , qt0 , qt1 , orient_t , cross_t;

	//	glLineWidth(4);
	//	glBegin(GL_LINES);
		float percent = 1.0 / DIVIDE_LINE;
		float t = 0;
		//t -= percent;
		Vector4f MT;
		switch (type_spline) {
		case spline_Linear:
			qt = (1 - t) * pos_p1 + t * pos_p2;
			break;
		case spline_CardinalCubic:
			MT = matrix4x4x4x1(Cardinal, Vector4f{ t*t*t, t*t, t, 1 });
			qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
			break;
		case spline_CubicB_Spline:
			MT = matrix4x4x4x1(B_Spline, Vector4f{ t*t*t, t*t, t, 1 });
			qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
			break;
		}
		for (int j = 0; j < DIVIDE_LINE; j++) {
			qt0 = qt;
			switch (type_spline) {
			case spline_Linear:
				orient_t = (1 - t) * orient_p1 + t * orient_p2;
				break;
			case spline_CardinalCubic:
				MT = matrix4x4x4x1(Cardinal, Vector4f{ t*t*t, t*t, t, 1 });
				orient_t = matrix3x4x4x1(orient_p1, orient_p2, orient_p3, orient_p4, MT);
				break;
			case spline_CubicB_Spline:
				MT = matrix4x4x4x1(B_Spline, Vector4f{ t*t*t, t*t, t, 1 });
				orient_t = matrix3x4x4x1(orient_p1, orient_p2, orient_p3, orient_p4, MT);
				break;
			}

			t += percent;
			switch (type_spline) {
			case spline_Linear:
				qt = (1 - t) * pos_p1 + t * pos_p2;
				break;
			case spline_CardinalCubic:
				 MT = matrix4x4x4x1(Cardinal, Vector4f{ t*t*t, t*t, t, 1 });
				qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4,MT);
				break;
			case spline_CubicB_Spline:
				MT = matrix4x4x4x1(B_Spline, Vector4f{ t*t*t, t*t, t, 1 });
				qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
				break;
			}
			qt1 = qt;
			lengthcount += sqrt((qt1.x - qt0.x)*(qt1.x - qt0.x) + (qt1.y - qt0.y)*(qt1.y - qt0.y) + (qt1.z - qt0.z)*(qt1.z - qt0.z));
			//畫鐵軌
			switch (type_track) {  
			case track_line:  //單軌
				glLineWidth(3);
				glBegin(GL_LINES);
				if (!doingShadows) {
					glColor3ub(32, 32, 64);
				}
				glVertex3f(qt0.x, qt0.y, qt0.z);
				glVertex3f(qt1.x, qt1.y, qt1.z);
				glEnd();
				break;
			case track_track:  //雙軌	
				orient_t.normalize();
				cross_t = (qt1 + (-1 * qt0)) * orient_t;
				cross_t.normalize();
				cross_t = cross_t * 2.5f;
				glLineWidth(3);
				glBegin(GL_LINES);
				if (!doingShadows) {
					glColor3ub(32, 32, 64);
				}
				glVertex3f(qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z);
				glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
				glVertex3f(qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z);
				glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
				glEnd();
			}
		 //畫枕木
			if (lengthcount > 5) {
				cross_t = (qt1 + (-1 * qt0)) * orient_t;
				cross_t.normalize();
				//cout << orient_t.x << " " << orient_t.y << " " << orient_t.z << endl;
				Pnt3f front = orient_t * cross_t;
				cross_t = cross_t * 4;
				front.normalize();
				glBegin(GL_QUADS);
				if (!doingShadows) {
					glColor3ub(94, 38, 18);
				}
				glVertex3f(qt1.x + cross_t.x + front.x, qt1.y + cross_t.y + front.y, qt1.z + cross_t.z + front.z);
				glVertex3f(qt1.x - cross_t.x + front.x, qt1.y - cross_t.y + front.y, qt1.z - cross_t.z + front.z);
				glVertex3f(qt1.x - cross_t.x - front.x, qt1.y - cross_t.y - front.y, qt1.z - cross_t.z - front.z);
				glVertex3f(qt1.x + cross_t.x - front.x, qt1.y + cross_t.y - front.y, qt1.z + cross_t.z - front.z);
				glEnd();
				lengthcount -= 5;
		    }
		}
	}
	//cout << t_time << endl;
	if (isrun) t_time += 0.001;
	if (t_time >1) t_time -= 1;

	//###################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}
void TrainView::drawTrain(float t , bool doingShadows) {
	spline_t type_spline = (spline_t)curve;
	int i = 0;
	while (t >= trackpercent[i]) {
		t -= trackpercent[i];
		i++;
	}
	//cout << "i = " << i << endl;
	//i = (i + m_pTrack->points.size() - 1) % m_pTrack->points.size();
	t = t / trackpercent[i];
	//cout << t << endl;
	//cout << t << endl;
	/*
	Pnt3f pos_p1 = m_pTrack->points[i].pos;                //起點的pos
	Pnt3f pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;      //終點的pos
	Pnt3f pos_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;      //終點的pos
	Pnt3f pos_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].pos;      //終點的pos
	Pnt3f orient_p1 = m_pTrack->points[i].orient;
	Pnt3f orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
	Pnt3f orient_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;
	Pnt3f orient_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].orient;
	Pnt3f qt, qt0, qt1 ,orient_t , cross_t , front;
	*/
	Pnt3f pos_p1 = m_pTrack->points[(i + m_pTrack->points.size()-1)% m_pTrack->points.size()].pos;                //起點的pos
	Pnt3f pos_p2 = m_pTrack->points[i % m_pTrack->points.size()].pos;      //終點的pos
	Pnt3f pos_p3 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;      //終點的pos
	Pnt3f pos_p4 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;      //終點的pos
	Pnt3f orient_p1 = m_pTrack->points[(i + m_pTrack->points.size()-1) % m_pTrack->points.size()].orient;
	Pnt3f orient_p2 = m_pTrack->points[i  % m_pTrack->points.size()].orient;
	Pnt3f orient_p3 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
	Pnt3f orient_p4 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;
	Pnt3f qt, qt0, qt1, orient_t, cross_t, front;
	float percent = 1.0 / DIVIDE_LINE;

	Vector4f MT;
	float length_temp = 0;  //站存累加長度
	float t2 = 0;
	while (length_temp < partLength[i] * t)
	{
		switch (type_spline) {
		case spline_Linear:
			qt = (1 - t2) * pos_p1 + t2 * pos_p2;
			orient_t = (1 - t2) * orient_p1 + t2 * orient_p2;
			break;
		case spline_CardinalCubic:
			MT = matrix4x4x4x1(Cardinal, Vector4f{ t2*t2*t2, t2*t2, t2, 1 });
			qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
			orient_t = matrix3x4x4x1(orient_p1, orient_p2, orient_p3, orient_p4, MT);
			break;
		case spline_CubicB_Spline:
			MT = matrix4x4x4x1(B_Spline, Vector4f{ t2*t2*t2, t2*t2, t2, 1 });
			qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
			orient_t = matrix3x4x4x1(orient_p1, orient_p2, orient_p3, orient_p4, MT);
			break;
		}
		qt0 = qt;
		t2 += percent;
		switch (type_spline) {
		case spline_Linear:
			qt = (1 - t2) * pos_p1 + t2 * pos_p2;
			break;
		case spline_CardinalCubic:
			MT = matrix4x4x4x1(Cardinal, Vector4f{ t2*t2*t2, t2*t2, t2, 1 });
			qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
			break;
		case spline_CubicB_Spline:
			MT = matrix4x4x4x1(B_Spline, Vector4f{ t2*t2*t2, t2*t2, t2, 1 });
			qt = matrix3x4x4x1(pos_p1, pos_p2, pos_p3, pos_p4, MT);
			break;
		}
		qt1 = qt;
		length_temp += sqrt((qt1.x - qt0.x)*(qt1.x - qt0.x) + (qt1.y - qt0.y)*(qt1.y - qt0.y) + (qt1.z - qt0.z)*(qt1.z - qt0.z));
	}
	//trainpos_t += sqrt((qt1.x - qt0.x)*(qt1.x - qt0.x) + (qt1.y - qt0.y)*(qt1.y - qt0.y) + (qt1.z - qt0.z)*(qt1.z - qt0.z));

		cross_t = (qt1 + (-1 * qt0)) * orient_t;
		cross_t.normalize();
		cross_t = cross_t * 3;       //寬度
		front = orient_t * cross_t;
		front.normalize();
		front = front * 5;           //長度
		orient_t.normalize();
		orient_t = orient_t * 5;     //高度
		if (!doingShadows) {
			glColor3ub(255, 0, 0);
		}
		glBegin(GL_QUADS);
		//上面
		glVertex3f(qt0.x + front.x + cross_t.x + orient_t.x, qt0.y + front.y + cross_t.y + orient_t.y, qt0.z + front.z + cross_t.z + orient_t.z);
		glVertex3f(qt0.x + front.x - cross_t.x + orient_t.x, qt0.y + front.y - cross_t.y + orient_t.y, qt0.z + front.z - cross_t.z + orient_t.z);
		glVertex3f(qt0.x - front.x - cross_t.x + orient_t.x, qt0.y - front.y - cross_t.y + orient_t.y, qt0.z - front.z - cross_t.z + orient_t.z);
		glVertex3f(qt0.x - front.x + cross_t.x + orient_t.x, qt0.y - front.y + cross_t.y + orient_t.y, qt0.z - front.z + cross_t.z + orient_t.z);
		//下面
		glVertex3f(qt0.x + front.x + cross_t.x, qt0.y + front.y + cross_t.y, qt0.z + front.z + cross_t.z);
		glVertex3f(qt0.x + front.x - cross_t.x, qt0.y + front.y - cross_t.y, qt0.z + front.z - cross_t.z);
		glVertex3f(qt0.x - front.x - cross_t.x, qt0.y - front.y - cross_t.y, qt0.z - front.z - cross_t.z);
		glVertex3f(qt0.x - front.x + cross_t.x, qt0.y - front.y + cross_t.y, qt0.z - front.z + cross_t.z);
		//右邊
		glVertex3f(qt0.x + front.x + cross_t.x + orient_t.x, qt0.y + front.y + cross_t.y + orient_t.y, qt0.z + front.z + cross_t.z + orient_t.z);
		glVertex3f(qt0.x - front.x + cross_t.x + orient_t.x, qt0.y - front.y + cross_t.y + orient_t.y, qt0.z - front.z + cross_t.z + orient_t.z);
		glVertex3f(qt0.x - front.x + cross_t.x, qt0.y - front.y + cross_t.y, qt0.z - front.z + cross_t.z);
		glVertex3f(qt0.x + front.x + cross_t.x, qt0.y + front.y + cross_t.y, qt0.z + front.z + cross_t.z);
		//左邊
		glVertex3f(qt0.x + front.x - cross_t.x + orient_t.x, qt0.y + front.y - cross_t.y + orient_t.y, qt0.z + front.z - cross_t.z + orient_t.z);
		glVertex3f(qt0.x - front.x - cross_t.x + orient_t.x, qt0.y - front.y - cross_t.y + orient_t.y, qt0.z - front.z - cross_t.z + orient_t.z);
		glVertex3f(qt0.x - front.x - cross_t.x, qt0.y - front.y - cross_t.y, qt0.z - front.z - cross_t.z);
		glVertex3f(qt0.x + front.x - cross_t.x, qt0.y + front.y - cross_t.y, qt0.z + front.z - cross_t.z);
		//前面
		glVertex3f(qt0.x + front.x + cross_t.x + orient_t.x, qt0.y + front.y + cross_t.y + orient_t.y, qt0.z + front.z + cross_t.z + orient_t.z);
		glVertex3f(qt0.x + front.x - cross_t.x + orient_t.x, qt0.y + front.y - cross_t.y + orient_t.y, qt0.z + front.z - cross_t.z + orient_t.z);
		glVertex3f(qt0.x + front.x - cross_t.x, qt0.y + front.y - cross_t.y, qt0.z + front.z - cross_t.z);
		glVertex3f(qt0.x + front.x + cross_t.x, qt0.y + front.y + cross_t.y, qt0.z + front.z + cross_t.z);
		//後面
		glVertex3f(qt0.x - front.x + cross_t.x + orient_t.x, qt0.y - front.y + cross_t.y + orient_t.y, qt0.z - front.z + cross_t.z + orient_t.z);
		glVertex3f(qt0.x - front.x - cross_t.x + orient_t.x, qt0.y - front.y - cross_t.y + orient_t.y, qt0.z - front.z - cross_t.z + orient_t.z);
		glVertex3f(qt0.x - front.x - cross_t.x, qt0.y - front.y - cross_t.y, qt0.z - front.z - cross_t.z);
		glVertex3f(qt0.x - front.x + cross_t.x, qt0.y - front.y + cross_t.y, qt0.z - front.z + cross_t.z);
		glEnd();
	
}
void TrainView::
	doPick(int mx, int my)
	//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	makeCurrent();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();

	gluPickMatrix((double)mx, (double)(viewport[3]-my), 
		5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100,buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);


	// draw the cubes, loading the names as we go
	for(size_t i=0; i<m_pTrack->points.size(); ++i) {
		glLoadName((GLuint) (i+1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3]-1;
	} else // nothing hit, nothing selected
		selectedCube = -1;
}
