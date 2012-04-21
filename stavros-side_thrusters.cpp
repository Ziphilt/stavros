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
  Thrusters();
  Thrusters(double,double);
  double left;
  double right;
};

Thrusters AI(cpBody*);
void circle(cpVect, double, double, Thrusters);
cpVect openGLpos(int, int);
const int circleSamples = 32;
GLdouble circleVAR[circleSamples*2 + 4];
double gravity = -10;
double force = 20;
//cpVect dest = cpv(0,0);
//}}}

//{{{ Main
int main()
{
  //{{{ Initializations
  int windowX = 680;
  int windowY = 680;
  sf::Window App(sf::VideoMode(windowX, windowY, 32), "Stavros");
  App.SetFramerateLimit(100);
  sf::Clock Clock;
  cerr.precision(3);
  //cerr << std::ios_base::scientific;

  // set window coordinates
  double x0 = -10.;
  double x1 = 10.;
  double y0 = -10.;
  double y1 = 10.;
  double ratio = (y1-y0)/(x1-x0);

  bool a     = false;
  bool e     = false;
  cpVect thrust = cpv(0,force);
  cpVect playerLThrust;
  cpVect playerRThrust;
  cpVect aiLThrust;
  cpVect aiRThrust;
  /*
  cpVect thrust1;
  cpVect thrust2;
  cpVect leftThrust = cpv(-force,0);
  cpVect rightThrust = cpv(force,0);
  cpVect upThrust = cpv(0,force);
  cpVect downThrust = cpv(0,-force); 
  double angThrust1; 
  double angThrust2;
  double torque = 10;
  double aTorque = torque;
  double eTorque = -torque;
  */
  
  cpSpace* space = cpSpaceNew();
  cpSpaceSetGravity(space, cpv(0, gravity));
  
  cpShape* wall0 = cpSegmentShapeNew(space->staticBody, cpv(10, -10), cpv(10, 10), 0);  
  cpShape* wall1 = cpSegmentShapeNew(space->staticBody, cpv(10, 10), cpv(-10, 10), 0);  
  cpShape* wall2 = cpSegmentShapeNew(space->staticBody, cpv(-10, 10), cpv(-10, -10), 0);  
  cpShape* wall3 = cpSegmentShapeNew(space->staticBody, cpv(-10, -10), cpv(10, -10), 0);
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
  cpFloat mass = 1;
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);

  cpBody* aiBallBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
  cpBodySetPos(aiBallBody, cpv(0, -8));
  cpShape* aiBallShape = cpSpaceAddShape(space, cpCircleShapeNew(aiBallBody, radius, cpvzero));
  cpShapeSetFriction(aiBallShape, 1.);
  cpShapeSetElasticity(aiBallShape, 0.5);

  cpBody* playerBallBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
  cpBodySetPos(playerBallBody, cpv(5, -9));
  cpShape* playerBallShape = cpSpaceAddShape(space, cpCircleShapeNew(playerBallBody, radius, cpvzero));
  cpShapeSetFriction(playerBallShape, 1.);
  cpShapeSetElasticity(playerBallShape, 0.5);

  
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
      /*
      if (Event.Type == sf::Event::MouseButtonPressed)
        dest = openGLpos(Event.MouseButton.X, Event.MouseButton.Y);
      */
      
      // keep aspect ratio
      if (Event.Type == sf::Event::Resized){
        double height = Event.Size.Height;
        double width  = Event.Size.Width;
        double windowRatio = height / width;
        if (windowRatio > ratio)       {windowX = width;          windowY = width * ratio;}
        else if (windowRatio < ratio)  {windowX = height / ratio; windowY = height;}
        else if (windowRatio == ratio) {windowX = width;          windowY = height;}
        glViewport(0, 0, windowX, windowY);
      }
    }
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
    //cpVect appliedForce = cpBodyGetForce(aiBallBody);

    Thrusters tAI = AI(aiBallBody);
    aiLThrust = thrust*tAI.left;
    aiRThrust = thrust*tAI.right;
    cpBodyResetForces(aiBallBody);
    cpBodyApplyForce(aiBallBody, cpvrotate(aiLThrust, aiBallBody->rot), cpvrotate(cpv(-radius,0), aiBallBody->rot));
    cpBodyApplyForce(aiBallBody, cpvrotate(aiRThrust, aiBallBody->rot), cpvrotate(cpv(radius,0), aiBallBody->rot));
   
    Thrusters tPlayer((double)a,(double)e); 
    playerLThrust = thrust*(cpFloat)a;
    playerRThrust = thrust*(cpFloat)e;
    cpBodyResetForces(playerBallBody);
    cpBodyApplyForce(playerBallBody, cpvrotate(playerLThrust, playerBallBody->rot), cpvrotate(cpv(-radius,0), playerBallBody->rot));
    cpBodyApplyForce(playerBallBody, cpvrotate(playerRThrust, playerBallBody->rot), cpvrotate(cpv(radius,0), playerBallBody->rot));

    /*
    thrust2 = leftThrust*(cpFloat)left + rightThrust*(cpFloat)right + upThrust*(cpFloat)up + downThrust*(cpFloat)down;
    Thrusters t2((double)left,(double)right,(double)up,(double)down,(double)a,(double)e);
    angThrust2 = aTorque*(cpFloat)a + eTorque*(cpFloat)e;
    cpBodyResetForces(playerBallBody);
    cpBodyApplyForce(playerBallBody, cpvrotate(thrust2, playerBallBody->rot), cpvzero);
    cpBodySetTorque(playerBallBody, angThrust2);
    */

    // cerr << 1/App.GetFrameTime() << endl;
    double dt = App.GetFrameTime();
    
    cpSpaceStep(space,dt);

    /*
    // draw destination reticle
    glColor3d(0,1,0);
    glBegin(GL_LINES);
    glVertex2d(dest.x - 3,dest.y);
    glVertex2d(dest.x - 2,dest.y);
    glVertex2d(dest.x + 2,dest.y);
    glVertex2d(dest.x + 3,dest.y);

    glVertex2d(dest.x,dest.y - 3);
    glVertex2d(dest.x,dest.y - 2); 
    glVertex2d(dest.x,dest.y + 2);
    glVertex2d(dest.x,dest.y + 3);
    glEnd();
    */

    circle(cpBodyGetPos(aiBallBody), cpBodyGetAngle(aiBallBody), radius, tAI);
    circle(cpBodyGetPos(playerBallBody), cpBodyGetAngle(playerBallBody), radius, tPlayer);

    App.Display();
  }
  //}}}

  cpShapeFree(aiBallShape);
  cpBodyFree(aiBallBody);
  cpShapeFree(playerBallShape);
  cpBodyFree(playerBallBody);
  cpShapeFree(wall0);
  cpShapeFree(wall1);
  cpShapeFree(wall2);
  cpShapeFree(wall3);
  cpSpaceFree(space);
  return EXIT_SUCCESS;
}
//}}}

//{{{ Functions
Thrusters AI(cpBody* b){ 
  //cpVect pos = cpBodyGetPos(b);
  double ang = cpBodyGetAngle(b);
  cpVect vel = cpBodyGetVel(b);
  double angVel = cpBodyGetAngVel(b);
  double m = cpBodyGetMass(b);
  Thrusters t(0,0);
  double gravT = -gravity*m/(force*2);
  t.left = gravT;
  t.right = gravT;
  /*
  double k = 4;
  double damp = 0.4;

  // control angle
  double rot = cpfclamp(-k*ang - damp*angVel,-1,1);
  */

  /*
  //double maxDownA = -force/m + gravity; // falling
  //double maxUpA = force/m + gravity; // ascending
  //double maxLatA = force/m;
  //if (maxUpA < 0) cerr << "Thrust not powerful enough!" << endl;
  // thruster setting appropriate for opposing gravity
  //double gravT = -gravity*m/force;
  cpVect d = cpv(dest.x - pos.x, dest.y - pos.y);
  // acceleration required to stop at dest
  cpVect requiredA = cpv(-vel.x*vel.x/(2*d.x), -vel.y*vel.y/(2*d.y));
  double err = 0.1;
  double k = 4;
  double damp = 0.4;

  // control angle
  double rot = cpfclamp(-k*ang - damp*angVel,-1,1);
  (rot > 0) ? t.rotL = rot : t.rotR = -rot;

  // control x position
  if (abs(d.x) < err && abs(vel.x) < err){
    double lat = cpfclamp(k*d.x - damp*vel.x,-1,1); // hold position
    (lat > 0) ? t.right = lat : t.left = -lat;
  }
  else if (d.x > 0){
    if (vel.x > 0 && abs(requiredA.x) > maxLatA)t.left = 1;
    else t.right = 1;
  }
  else if (d.x < 0){
    if (vel.x < 0 && abs(requiredA.x) > maxLatA)t.right = 1;
    else t.left = 1;
  }

  // control y position
  if (abs(d.y) < err && abs(vel.y) < err){
    t.up = cpfclamp(gravT + k*d.y - damp*vel.y,0,1); // hover
  }
  else if (d.y > 0){
    if (vel.y > 0 && requiredA.y < maxDownA) t.down = 1;
    else t.up = 1;
  }
  else if (d.y < 0){
    if (vel.y < 0 && requiredA.y > maxUpA) t.up = 1;
    else t.down = 1;
  }
  */
  return t;
}

//{{{ draw a circle
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
      glVertex2d(-1,-t.left);

      glVertex2d(1,0);
      glVertex2d(1,-t.right);
    glEnd();
  glPopMatrix();
}//}}}

cpVect openGLpos(int x, int y){
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  GLfloat winX, winY;
  GLdouble posX, posY, posZ;

  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  winX = (float)x;
  winY = (float)viewport[3] - (float)y;
  //glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

  gluUnProject( winX, winY, 0, modelview, projection, viewport, &posX, &posY, &posZ);

  return cpv(posX, posY);
}

Thrusters::Thrusters(){}
Thrusters::Thrusters(double le,double ri): left(le),right(ri) {}
//}}}
