//{{{ Includes
#include <SFML/Graphics.hpp>
#include <chipmunk/chipmunk.h>
#include <iostream>
using std::cerr;
using std::endl;
#include <vector>
using std::vector;
#include <cmath>
//}}}

//{{{ Declarations
double lin(double, double, double);
void circle(cpVect, double, double);
double castFloat(int);
//}}}

//{{{ Main
int main()
{
  sf::Window App(sf::VideoMode(680, 680, 32), "Stavros");
  App.SetFramerateLimit(100);
  sf::Clock Clock;

  // set window coordinates
  double x0 = -20.;
  double x1 = 20.;
  double y0 = -20.;
  double y1 = 20.;
  double ratio = (y1-y0)/(x1-x0);
  
  cpSpace* space = cpSpaceNew();
  cpSpaceSetGravity(space, cpv(0,-100));
  cpShape *ground1 = cpSegmentShapeNew(space->staticBody, cpv(-20, 5), cpv(0, 0), 0);
  cpShapeSetFriction(ground1, 1);
  cpSpaceAddShape(space, ground1);
  cpShape *ground2 = cpSegmentShapeNew(space->staticBody, cpv(0, 0), cpv(20, 5), 0);
  cpShapeSetFriction(ground2, 1);
  cpSpaceAddShape(space, ground2);
  cpFloat radius = 5;
  cpFloat mass = 1;
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
  cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
  cpBodySetPos(ballBody, cpv(7, 15));
  cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
  cpShapeSetFriction(ballShape, 0.7);

  //{{{ OpenGL stuff
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(x0, x1, y0, y1);
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0., 0., 0., 0.);
  // antialiasing
  glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_POINT_SMOOTH_HINT, GL_DONT_CARE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //}}}

  //{{{ Main Loop
  while (App.IsOpened()){
    //{{{ Event processing 
    sf::Event Event;
    while (App.GetEvent(Event)){
      if (Event.Type == sf::Event::Closed)
          App.Close();
      if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Escape))
          App.Close();
      // keep aspect ratio
      if (Event.Type == sf::Event::Resized){
        double height = Event.Size.Height;
        double width  = Event.Size.Width;
        double windowRatio = height / width;
        if (windowRatio > ratio)
          glViewport(0, 0, width, width * ratio);
        else if (windowRatio < ratio)
          glViewport(0, 0, height / ratio, height);
        else if (windowRatio == ratio)
          glViewport(0, 0, width, height);
        else cerr << "something weird, windowRatio = " << windowRatio << ", width = " << width << ", height = " << height << endl; 
      }
    }
    //}}}

    App.SetActive();
    glClear(GL_COLOR_BUFFER_BIT);

    //{{{ draw axes
    glBegin(GL_LINES);
    glColor3d(.7,.7,.7);
    glVertex2d(0.,-32.);
    glVertex2d(0.,32.);
    glVertex2d(-32.,0.);
    glVertex2d(32.,0.);
    glEnd();
    //}}}

    // cerr << 1/App.GetFrameTime() << endl;
    double dt = App.GetFrameTime();
    
    cpSpaceStep(space,dt);
    cpVect pos = cpBodyGetPos(ballBody);
    glColor3d(1,1,1);
    glBegin(GL_LINE_STRIP);
    glVertex2d(-20,5);
    glVertex2d(0,0);
    glVertex2d(20,5);
    glEnd();
    circle(pos, cpBodyGetAngle(ballBody), radius);

    App.Display();
  }
  //}}}

  cpShapeFree(ballShape);
  cpBodyFree(ballBody);
  cpShapeFree(ground1);
  cpShapeFree(ground2);
  cpSpaceFree(space);
  return EXIT_SUCCESS;
}
//}}}

//{{{ Functions
double lin(double t0, double t1, double t){
  return t0 * t + t1 * (1-t);
}

void circle(cpVect pos, double angle, double radius){
  double x = pos.x;
  double y = pos.y;
  glColor3d(0.3,0,0);
  glBegin(GL_TRIANGLE_FAN);
  for(int t = 0; t != 64; t++){
    double a = t * 0.098175;
    glVertex2d(x+radius*cos(a),y+radius*sin(a));
  }
  glEnd();
  glColor3d(1,0,0);
  glBegin(GL_LINE_STRIP);
  for(int t = 0; t != 65; t++){
    double a = t * 0.098175;
    glVertex2d(x+radius*cos(a + angle),y+radius*sin(a + angle));
  }
  glVertex2d(x,y);
  glEnd();
}

double castFloat(int i){return i;}
//}}}
