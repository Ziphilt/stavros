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
double f(double);
double fx(double, double);
double fy(double, double);
double lin(double, double, double);
void circle(double, double, double);
double castFloat(int);
//}}}

//{{{ Main
int main()
{
  sf::Window App(sf::VideoMode(1200, 700, 32), "Stavros Plotting");
  App.SetFramerateLimit(100);
  sf::Clock Clock;

  // set window coordinates
  double x0 = -1.71429;
  double x1 = 1.71429;
  double y0 = -1.;
  double y1 = 1.;
  double ratio = (y1-y0)/(x1-x0);

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

  //{{{ function plotty stuff
  double t0 = 0;
  double t1 = 1;
  vector<double> xpoints;
  vector<double> ypoints;
  double time;
  int samples = 10000;
  //}}}
  
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

    xpoints.clear();
    ypoints.clear();
    
    time = Clock.GetElapsedTime();
    /*
    if (time > 11)
      time = 11;
    samples = ceil(exp(time));
    cerr << samples << endl;
    */
    for(int i = 0; i != samples; i++){
      double a = castFloat(i)/castFloat(samples);
      double t = lin(t0, t1, a);  
      xpoints.push_back(fx(t, time));
      ypoints.push_back(fy(t, time));
    }

    //{{{ draw axes
    glBegin(GL_LINES);
    glColor3d(.7,.7,.7);
    glVertex2d(0.,-32.);
    glVertex2d(0.,32.);
    glVertex2d(-32.,0.);
    glVertex2d(32.,0.);
    glEnd();
    //}}}

    //{{{ function drawy stuff
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i != samples; i++){
      double a = castFloat(i)/castFloat(samples);
      glColor3d(1, 1-a, a);
      glVertex2d(xpoints[i],ypoints[i]);
    }
    glEnd();
    // draw a spinning circle
    glColor3d(1,1,1);
    circle(cos(time*3),sin(time*3),0.01);
    //}}}
    App.Display();
  }
  //}}}

  return EXIT_SUCCESS;
}
//}}}

//{{{ Functions

double f(double x){
  return sin(60*x);
  //double a = t/5;
  //return exp(-x*x/(2*a*a))/(a*sqrt(2*3.14159));
}
double fx(double t, double time){
  //return t;
  ///*
  double px0 = 0;
  double px1 = cos(time*3);
  double px2 = 1;
  return lin(lin(px0,px1,t),lin(px1,px2,t),t);
  //*/
}

double fy(double t, double time){
  //return sin(exp(time)*t);
  ///*
  double py0 = 0;
  double py1 = sin(time*3);
  double py2 = 1;
  return lin(lin(py0,py1,t),lin(py1,py2,t),t);
  //*/
}

double lin(double t0, double t1, double t){
  return t0 * t + t1 * (1-t);
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
