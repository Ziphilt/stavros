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
  float x;
  float y;
  Vect negate();
  void rotate(float);
  float resultant();
};

struct Ball{
  float m;
  float r;
  Vect p;
  Vect v;
  void draw(float, float, float);
  void move(float);
  void collideWalls();
};

float f(float);
void circle(float, float, float);
float castFloat(int);
Vect fromPolar(float, float);
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
  float x0 = 0.;
  float x1 = 2.;
  float y0 = 0.;
  float y1 = 2.;
  float ratio = (y1-y0)/(x1-x0);

  Ball a;
  a.m = 1;
  a.r = 0.1;
  a.p.x = 0.4;
  a.p.y = 1.1;
  a.v.x = 0.;
  a.v.y = 0.;

  Ball b;
  b.m = 9;
  b.r = 0.3;
  b.p.x = 1.4;
  b.p.y = 1;
  b.v.x = -1.;
  b.v.y = 0.;

  //{{{ OpenGL stuff
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(x0, x1, y0, y1, 0, 1);
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0.f, 0.f, 0.f, 0.f);
  //}}}

  /*
  // evaluate f(x) on a bunch of points
  vector<float> xpoints;
  vector<float> ypoints;

  int samples = 1000;
  for(int i = 0; i != samples; i++){
    float a = castFloat(i)/castFloat(samples);
    // linear interpolation
    float x = x0 * a + x1 * (1-a);
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
        float height = Event.Size.Height;
        float width  = Event.Size.Width;
        float windowRatio = height / width;
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
    glColor3f(.7,.7,.7);
    glVertex2f(0.,-32.);
    glVertex2f(0.,32.);
    glVertex2f(-32.,0.);
    glVertex2f(32.,0.);
    glEnd();
    //}}}

    /*
    // draw f(x)
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i != samples; i++){
      float a = castFloat(i)/castFloat(samples);
      glColor3f(1., 1-a, a);
      glVertex2f(xpoints[i],ypoints[i]);
    }
    glEnd();

    // draw a spinning circle
    float time = Clock.GetElapsedTime();
    circle(0.5*cos(time),0.5*sin(time),0.1);
    */

    // cerr << 1/App.GetFrameTime() << endl;

    float dp = vectAdd(a.p, b.p.negate()).resultant();

    if (dp < a.r + b.r){
      cerr << "collision occured ";
      elasticCollision(a,b);
      cerr << dp - a.r - b.r << endl;
    }

    a.collideWalls();
    b.collideWalls();

    float dt = App.GetFrameTime();
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

void Vect::rotate(float t){
  float x1 = x * cos(t) - y * sin(t);
  float y1 = x * sin(t) + y * cos(t);
  x = x1;
  y = y1;
}

float Vect::resultant(){
  return sqrt(y*y + x*x);
}

Vect fromPolar(float r, float t){
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
void Ball::draw(float red, float green, float blue){
  glColor3f(red, green, blue);
  circle(p.x,p.y,r);
}

void Ball::move(float dt){
  p.x = p.x + dt * v.x;
  p.y = p.y + dt * v.y;
}

void Ball::collideWalls(){
  // coefficient of restitution
  float c = 1;
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
  Vect dp;
  dp.x = b.p.x - a.p.x;
  dp.y = b.p.y - a.p.y;
  float tc = atan2(dp.y, dp.x);
  a.v.rotate(-tc);
  b.v.rotate(-tc);
  float M = a.m + b.m;
  float avf = (a.v.x * (a.m - b.m) + 2 * b.m * b.v.x) / M;
  float bvf = (b.v.x * (b.m - a.m) + 2 * a.m * a.v.x) / M;
  a.v.x = avf;
  b.v.x = bvf;
  a.v.rotate(tc);
  b.v.rotate(tc);
  /* intended to move the balls apart
   * it doesn't work
  float d = sqrt(dp.y*dp.y + dp.x*dp.x) - a.r - b.r;
  Vect adjust;
  adjust = fromPolar(d, tc);
  b.p.x += adjust.x*2;
  b.p.y += adjust.y*2; */
}
//}}}

float f(float x){
  return exp(-pow(x,2));
}

void circle(float x, float y, float r){
//  glColor3f(8., .4, 0.);
  glBegin(GL_POLYGON);
  for(int t = 0; t != 32; t++){
    float a = t * 0.19635;
    glVertex2f(x+r*cos(a),y+r*sin(a));
  }
  glEnd();
}

float castFloat(int i){return i;}

//}}}
