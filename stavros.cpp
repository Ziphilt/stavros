//{{{ Includes
#include <SFML/Graphics.hpp>
#include <iostream>
using std::cerr;
using std::endl;
#include <vector>
using std::vector;
#include <cmath>
//}}}

//{{{ Declarations
struct Vect{
  Vect();
  Vect(double, double);
  double x;
  double y;
};

struct Ball{
  Ball();
  Ball(double,double,double,double,double,double);
  double m;
  double r;
  Vect p;
  Vect v;
  void draw(double, double, double);
  void move(double);
  void collideWalls();
};

double f(double);
void circle(double, double, double);
double castFloat(int);

Vect fromPolar(double, double);
Vect vectAdd(const Vect&, const Vect&);
Vect negate(const Vect&);
Vect rotate(const Vect&, double);
double resultant(const Vect&);
void collision(Ball&, Ball&);
//#define elasticOn
#define gravityOn
#define frictionOn
//}}}

//{{{ Main
int main()
{
  sf::Window App(sf::VideoMode(512, 512, 32), "Stavros");
  App.SetFramerateLimit(100);
  sf::Clock Clock;

  // set window coordinates
  double x0 = 0.;
  double x1 = 2.;
  double y0 = 0.;
  double y1 = 2.;
  double ratio = (y1-y0)/(x1-x0);
                           //m,r,  p,      v
  vector<Ball> balls = {Ball(1,0.1,1.1,1.1,1,-1), Ball(1,0.1,1.3,1.3,1,-1), Ball(1,0.1,1.5,1.5,1,-1), Ball(1,0.1,1.7,1.7,1,-1),
                        Ball(1,0.1,1.1,0.9,1,-1), Ball(1,0.1,1.3,0.7,1,-1), Ball(1,0.1,1.5,0.5,1,-1), Ball(1,0.1,0.3,0.3,1,-1),
                        Ball(1,0.05,0.3,0.3,-1,1), Ball(1,0.05,0.5,0.5,-1,1), Ball(1,0.05,0.7,0.7,-1,1), Ball(1,0.05,0.9,0.9,-1,1),
                        Ball(10,0.2,0.5,1.5,0.3,-0.3)};

  //{{{ OpenGL stuff
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(x0, x1, y0, y1);
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0., 0., 0., 0.);
  //}}}

  /*{{{ function plotty stuff
  // evaluate f(x) on a bunch of points
  vector<double> xpoints;
  vector<double> ypoints;

  int samples = 1000;
  for(int i = 0; i != samples; i++){
    double a = castFloat(i)/castFloat(samples);
    // linear interpolation
    double x = x0 * a + x1 * (1-a);
    xpoints.push_back(x);
    ypoints.push_back(f(x));
  }
  }}}*/

  //{{{ Main Loop
  while (App.IsOpened())
  {
    //{{{ Event processing 
    sf::Event Event;
    while (App.GetEvent(Event))
    {
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

    /*{{{ function drawy stuff
    // draw f(x)
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i != samples; i++){
      double a = castFloat(i)/castFloat(samples);
      glColor3d(1., 1-a, a);
      glVertex2d(xpoints[i],ypoints[i]);
    }
    glEnd();

    // draw a spinning circle
    double time = Clock.GetElapsedTime();
    circle(0.5*cos(time),0.5*sin(time),0.1);
    }}}*/

    // cerr << 1/App.GetFrameTime() << endl;
    double dt = App.GetFrameTime();
    for(vector<Ball>::iterator j=balls.begin();j!=balls.end();j++){
      //int i = std::distance(balls.begin(),p);
      //cerr << i << j->m << endl;
      for(vector<Ball>::iterator q=balls.begin();q!=balls.end();q++){
        if (j != q){ // if they are not the same ball
          double dp = resultant(vectAdd(q->p, negate(j->p)));
          if (dp < j->r + q->r){
            collision(*j,*q);
          }
        }
      }
      j->collideWalls();
      j->move(dt);
      j->draw(0.5,0.6,0.9);
    }

    App.Display();
  }
  //}}}

  return EXIT_SUCCESS;
}
//}}}

//{{{ Functions

//{{{ Vect
Vect::Vect(): x(0), y(0){}

Vect::Vect(double xi, double yi): x(xi), y(yi){}

Vect negate(const Vect& v){
  Vect a(-v.x, -v.y);
  return a;
}

Vect rotate(const Vect& v, double t){
  Vect a(v.x * cos(t) - v.y * sin(t), v.x * sin(t) + v.y * cos(t));
  return a;
}

double resultant(const Vect& v){
  return sqrt(v.y*v.y + v.x*v.x);
}

Vect fromPolar(double r, double t){
  Vect a(r * cos(t), r * sin(t));
  return a;
}

Vect vectAdd(const Vect& v0, const Vect& v1){
  Vect a(v0.x + v1.x, v0.y + v1.y);
  return a;
}
//}}}

//{{{ Ball
Ball::Ball(){}
Ball::Ball(double mi,double ri,double px,double py,double vx,double vy): m(mi),r(ri),p(px,py),v(vx,vy) {}
void Ball::draw(double red, double green, double blue){
  glColor3d(red, green, blue);
  circle(p.x,p.y,r);
}

void Ball::move(double dt){
  #ifdef gravityOn
  double g = -2.; // gravitational constant
  #else
  double g = 0.;
  #endif
  #ifdef frictionOn
  double c = 1.;  // friction coefficient
  #else
  double c = 0.;
  #endif
  p.x = p.x + dt * v.x - dt*dt*c*v.x/2;
  v.x = v.x - dt * c*v.x;
  p.y = p.y + dt * v.y + dt*dt*(g - c*v.y)/2;
  v.y = v.y + dt * (g - c*v.y);
}

void Ball::collideWalls(){
  // coefficient of restitution
  #ifdef elasticOn
  double c = 1;
  #else
  double c = 0.9;
  #endif
  if (p.y - r <= 0){
    /*#ifdef gravityOn
      if (abs(v.y) <= 0.001)
        v.y = 0;
      else v.y = -v.y*c;
    //#endif*/
    v.y = -v.y*c;
    p.y = r;
  }
  if (p.x - r <= 0){
    v.x = -v.x*c;
    p.x = r;
  }
  if (p.y + r >= 2){
    v.y = -v.y*c;
    p.y = 2 - r;
  }
  if (p.x + r >= 2){
    v.x = -v.x*c;
    p.x = 2 - r;
  }
}

void collision(Ball& a, Ball& b){
  Vect dp = vectAdd(b.p, negate(a.p));
  double tc = atan2(dp.y, dp.x);
  a.v = rotate(a.v, -tc);
  b.v = rotate(b.v, -tc);
  double M = a.m + b.m;
  #ifdef elasticOn
  double c = 1;
  #else
  double c = 0.9;
  #endif
  double avf = (a.m*a.v.x + b.m*b.v.x + b.m*c*(b.v.x - a.v.x)) / M;
  double bvf = (b.m*b.v.x + a.m*a.v.x + a.m*c*(a.v.x - b.v.x)) / M;
  a.v.x = avf;
  b.v.x = bvf;
  a.v = rotate(a.v, tc);
  b.v = rotate(b.v, tc);
  // move the balls apart
  double d = resultant(dp) - a.r - b.r;
  Vect adjust_a = fromPolar(d * b.m / M, tc);
  Vect adjust_b = fromPolar(-d * a.m / M, tc);
  a.p = vectAdd(a.p, adjust_a);
  b.p = vectAdd(b.p, adjust_b);
}
//}}}

double f(double x){
  return exp(-pow(x,2));
}

void circle(double x, double y, double r){
//  glColor3d(8., .4, 0.);
  glBegin(GL_POLYGON);
  for(int t = 0; t != 64; t++){
    double a = t * 0.098175;
    glVertex2d(x+r*cos(a),y+r*sin(a));
  }
  glEnd();
}

double castFloat(int i){return i;}
//}}}
