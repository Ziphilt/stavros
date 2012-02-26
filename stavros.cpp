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
struct Thrusters {
  Thrusters(double,double,double,double,double,double);
  double left;
  double right;
  double up;
  double down;
  double rotL;
  double rotR;
};

double lin(double, double, double);
void circle(cpVect, double, double, Thrusters);
const int circleSamples = 32;
GLdouble circleVAR[circleSamples*2 + 4];
//}}}

//{{{ Main
int main()
{
  //{{{ Initializations
  sf::Window App(sf::VideoMode(680, 680, 32), "Stavros");
  App.SetFramerateLimit(100);
  sf::Clock Clock;

  // set window coordinates
  double x0 = -10.;
  double x1 = 10.;
  double y0 = -10.;
  double y1 = 10.;
  double ratio = (y1-y0)/(x1-x0);

  bool left  = false;
  bool right = false;
  bool up    = false;
  bool down  = false;
  bool a     = false;
  bool e     = false;
  cpVect thrust;
  double force = 10;
  cpVect leftThrust = cpv(-force,0);
  cpVect rightThrust = cpv(force,0);
  cpVect upThrust = cpv(0,force);
  cpVect downThrust = cpv(0,-force); 
  double angThrust;
  double torque = 10;
  double aTorque = torque;
  double eTorque = -torque;
  
  cpSpace* space = cpSpaceNew();
  //cpSpaceSetGravity(space, cpv(0,-10));
  
  cpShape *wall0 = cpSegmentShapeNew(space->staticBody, cpv(10, -10), cpv(10, 10), 0);  
  cpShape *wall1 = cpSegmentShapeNew(space->staticBody, cpv(10, 10), cpv(-10, 10), 0);  
  cpShape *wall2 = cpSegmentShapeNew(space->staticBody, cpv(-10, 10), cpv(-10, -10), 0);  
  cpShape *wall3 = cpSegmentShapeNew(space->staticBody, cpv(-10, -10), cpv(10, -10), 0);
  cpShapeSetFriction(wall0, 10);
  cpShapeSetFriction(wall1, 10);
  cpShapeSetFriction(wall2, 10);
  cpShapeSetFriction(wall3, 10);
  cpShapeSetElasticity(wall0, 0.8);
  cpShapeSetElasticity(wall1, 0.8);
  cpShapeSetElasticity(wall2, 0.8);
  cpShapeSetElasticity(wall3, 0.8);
  cpSpaceAddShape(space, wall0);
  cpSpaceAddShape(space, wall1);
  cpSpaceAddShape(space, wall2);
  cpSpaceAddShape(space, wall3);

  cpFloat radius = 1;
  cpFloat mass = 3;
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
  cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
  cpBodySetPos(ballBody, cpv(0, 1));
  cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
  cpShapeSetFriction(ballShape, 1.);
  cpShapeSetElasticity(ballShape, 0.5);
  
  for(int t = 0; t != circleSamples*2 + 2; t+=2){
    double a = t * M_PI / (double)circleSamples;
    circleVAR[t]   = cos(a + M_PI/2);
    circleVAR[t+1] = sin(a + M_PI/2);
  }
  circleVAR[circleSamples*2 + 2] = circleVAR[circleSamples*2 + 3] = 0;
  
  //{{{ OpenGL stuff
  glDisable(GL_DEPTH_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  // antialiasing
  glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(x0, x1, y0, y1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0., 0., 0., 0.);
  //}}}
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
    //cpVect appliedForce = cpBodyGetForce(ballBody);
    thrust = leftThrust*(cpFloat)left + rightThrust*(cpFloat)right + upThrust*(cpFloat)up + downThrust*(cpFloat)down;
    Thrusters t((double)left,(double)right,(double)up,(double)down,(double)a,(double)e);
    angThrust = aTorque*(cpFloat)a + eTorque*(cpFloat)e;
    cpBodyResetForces(ballBody);
    cpBodyApplyForce(ballBody, cpvrotate(thrust, ballBody->rot), cpvzero);
    cpBodySetTorque(ballBody, angThrust);

    // cerr << 1/App.GetFrameTime() << endl;
    double dt = App.GetFrameTime();
    
    cpSpaceStep(space,dt);
    circle(cpBodyGetPos(ballBody), cpBodyGetAngle(ballBody), radius, t);

    App.Display();
  }
  //}}}

  cpShapeFree(ballShape);
  cpBodyFree(ballBody);
  cpShapeFree(wall0);
  cpShapeFree(wall1);
  cpShapeFree(wall2);
  cpShapeFree(wall3);
  cpSpaceFree(space);
  return EXIT_SUCCESS;
}
//}}}

//{{{ Functions
double lin(double t0, double t1, double t){
  return t0 * t + t1 * (1-t);
}

void circle(cpVect pos, double angle, double radius, Thrusters t){
  glVertexPointer(2, GL_DOUBLE, 0, circleVAR);
  glPushMatrix();
    glTranslated(pos.x, pos.y, 0);
    glRotated(angle*180/M_PI, 0, 0, 1);
    glScaled(radius, radius, 1);
    // fill inside
    glColor3d(0.3, 0, 0); // hardcoded (TODO) color
    glDrawArrays(GL_TRIANGLE_FAN, 0, circleSamples);
    // draw outline and radius
    glColor3d(1,0,0); // hardcoded (TODO) color
    glDrawArrays(GL_LINE_STRIP, 0, circleSamples + 2);
    // draw thrusters
    glColor3d(0,1,1);
    glBegin(GL_LINES);
      glVertex2d(-1,0);
      glVertex2d(-1-t.right,0);

      glVertex2d(1,0);
      glVertex2d(1+t.left,0);

      glVertex2d(0,1);  
      glVertex2d(0,1+t.down);

      glVertex2d(0,-1);
      glVertex2d(0,-1-t.up);

      glVertex2d(0,1);
      glVertex2d(-t.rotR,1);
      glVertex2d(0,-1);   
      glVertex2d(t.rotR,-1);

      glVertex2d(0,1);
      glVertex2d(t.rotL,1);
      glVertex2d(0,-1);
      glVertex2d(-t.rotL,-1);
    glEnd();
  glPopMatrix();
}

Thrusters::Thrusters(double le,double ri,double u,double d,double rL,double rR): left(le),right(ri),up(u),down(d),rotL(rL),rotR(rR) {}
//}}}
