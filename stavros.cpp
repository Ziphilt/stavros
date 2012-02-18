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
  double x;
  double y;
  Vect negate();
  void rotate(double);
  double resultant();
};

struct Ball{
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
Vect vectAdd(Vect, Vect);
void elasticCollision(Ball&, Ball&);
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

  Ball a;
  a.m = 1.;
  a.r = 0.1;
  a.p.x = 0.4;
  a.p.y = 1.;
  a.v.x = 1.;
  a.v.y = 0.;

  Ball b;
  b.m = 9.;
  b.r = 0.3;
  b.p.x = 1.4;
  b.p.y = 1.;
  b.v.x = -1.;
  b.v.y = 0.;

  //{{{ OpenGL stuff
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(x0, x1, y0, y1, 0, 1);
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0., 0., 0., 0.);
  //}}}

  /*
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
  */

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

    /*
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
    */

    // cerr << 1/App.GetFrameTime() << endl;

    double dp = vectAdd(a.p, b.p.negate()).resultant();

    if (dp < a.r + b.r){
      cerr << "collision occured " << dp - a.r - b.r << endl;
      elasticCollision(a,b);
      dp = vectAdd(a.p, b.p.negate()).resultant();
      cerr << "after collision   " << dp - a.r - b.r << endl;
    }

    a.collideWalls();
    b.collideWalls();

    double dt = App.GetFrameTime();
    a.move(dt);
    a.draw(0., 1., 0.);
    b.move(dt);
    b.draw(1., 0., 0.);

    App.Display();
  }

  return EXIT_SUCCESS;
}
//}}}

//{{{ Functions

//{{{ Vect
Vect Vect::negate(){
  Vect v;
  v.x = -x;
  v.y = -y;
  return v;
}

void Vect::rotate(double t){
  double x1 = x * cos(t) - y * sin(t);
  double y1 = x * sin(t) + y * cos(t);
  x = x1;
  y = y1;
}

double Vect::resultant(){
  return sqrt(y*y + x*x);
}

Vect fromPolar(double r, double t){
  Vect v;
  v.x = r * cos(t);
  v.y = r * sin(t);
  return v;
}

Vect vectAdd(Vect a, Vect b){
  Vect v;
  v.x = a.x + b.x;
  v.y = a.y + b.y;
  return v;
}
//}}}

//{{{ Ball
void Ball::draw(double red, double green, double blue){
  glColor3d(red, green, blue);
  circle(p.x,p.y,r);
}

void Ball::move(double dt){
  p.x = p.x + dt * v.x;
  p.y = p.y + dt * v.y;
}

void Ball::collideWalls(){
  // coefficient of restitution
  double c = 1;
  if (p.y - r <= 0){
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

void elasticCollision(Ball& a, Ball& b){
  Vect dp = vectAdd(a.p.negate(), b.p);
  double tc = atan2(dp.y, dp.x);
  a.v.rotate(-tc);
  b.v.rotate(-tc);
  double M = a.m + b.m;
  double avf = (a.v.x * (a.m - b.m) + 2 * b.m * b.v.x) / M;
  double bvf = (b.v.x * (b.m - a.m) + 2 * a.m * a.v.x) / M;
  a.v.x = avf;
  b.v.x = bvf;
  a.v.rotate(tc);
  b.v.rotate(tc);
  // move the balls apart
  double d = dp.resultant() - a.r - b.r;
  Vect adjust;
  adjust = fromPolar(-d, tc);
  b.p.x += adjust.x;
  b.p.y += adjust.y; 
}
//}}}

double f(double x){
  return exp(-pow(x,2));
}

void circle(double x, double y, double r){
//  glColor3d(8., .4, 0.);
  glBegin(GL_POLYGON);
  for(int t = 0; t != 32; t++){
    double a = t * 0.19635;
    glVertex2d(x+r*cos(a),y+r*sin(a));
  }
  glEnd();
}

double castFloat(int i){return i;}

//}}}
