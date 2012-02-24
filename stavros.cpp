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
  double x0 = -40.;
  double x1 = 40.;
  double y0 = -40.;
  double y1 = 40.;
  double ratio = (y1-y0)/(x1-x0);

  bool left  = false;
  bool right = false;
  bool up    = false;
  bool down  = false;
  bool a     = false;
  bool e     = false;
  cpVect thrust;
  double force = 70;
  cpVect leftThrust = cpv(-force,0);
  cpVect rightThrust = cpv(force,0);
  cpVect upThrust = cpv(0,force);
  cpVect downThrust = cpv(0,-force); 
  double angThrust;
  double torque = 3;
  double aTorque = torque;
  double eTorque = -torque;
  
  cpSpace* space = cpSpaceNew();
  cpSpaceSetGravity(space, cpv(0,-10));
  cpShape *ground1 = cpSegmentShapeNew(space->staticBody, cpv(-20, 5), cpv(-5, 5), 0);
  cpShapeSetFriction(ground1, 10);
  cpShapeSetElasticity(ground1, 0.2);
  cpSpaceAddShape(space, ground1);
  cpShape *ground2 = cpSegmentShapeNew(space->staticBody, cpv(-5, 0), cpv(20, 0), 0);
  cpShapeSetFriction(ground2, 0.0);
  cpShapeSetElasticity(ground2, 0.5);
  //cpShapeSetSurfaceVelocity(ground2, cpv(10,0));
  cpSpaceAddShape(space, ground2);
  cpFloat radius = 1;
  cpFloat mass = 1;
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
  cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
  cpBodySetPos(ballBody, cpv(0, 1));
  cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
  cpShapeSetFriction(ballShape, 1.);
  cpShapeSetElasticity(ballShape, 0.5);

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
    left  = App.GetInput().IsKeyDown(sf::Key::Left);
    right = App.GetInput().IsKeyDown(sf::Key::Right);
    up    = App.GetInput().IsKeyDown(sf::Key::Up);
    down  = App.GetInput().IsKeyDown(sf::Key::Down);
    a     = App.GetInput().IsKeyDown(sf::Key::A);
    e     = App.GetInput().IsKeyDown(sf::Key::E);
    //}}}

    App.SetActive();
    glClear(GL_COLOR_BUFFER_BIT);

    /*{{{ draw axes
    glBegin(GL_LINES);
    glColor3d(.7,.7,.7);
    glVertex2d(0.,-32.);
    glVertex2d(0.,32.);
    glVertex2d(-32.,0.);
    glVertex2d(32.,0.);
    glEnd();
    }}}*/
    cpVect gPos = cpBodyLocal2World(ballBody, cpv(1,0));
    //cpVect appliedForce = cpBodyGetForce(ballBody);
    cerr << gPos.x << " " << gPos.y << endl;
    thrust = leftThrust*(cpFloat)left + rightThrust*(cpFloat)right + upThrust*(cpFloat)up + downThrust*(cpFloat)down;
    angThrust = aTorque*(cpFloat)a + eTorque*(cpFloat)e;
    cpBodyResetForces(ballBody);
    cpBodySetForce(ballBody, thrust);
    cpBodySetTorque(ballBody, angThrust);
    // cerr << 1/App.GetFrameTime() << endl;
    double dt = App.GetFrameTime();
    
    cpSpaceStep(space,dt);
    cpVect pos = cpBodyGetPos(ballBody);
    glColor3d(1,1,1);
    glBegin(GL_LINES);
    glVertex2d(-20,5);
    glVertex2d(-5,5);
    glVertex2d(-5,0);
    glVertex2d(20,0);
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
