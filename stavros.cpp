//{{{ Includes
#include <SFML/Window.hpp>
#include <iostream>
using std::cerr;
using std::endl;
#include <vector>
using std::vector;
#include <cmath>
//}}}

float f(float);
void circle(float, float, float);
float castFloat(int);

//{{{ main
int main()
{
  sf::Window App(sf::VideoMode(512, 512, 32), "Stavros");
  App.SetFramerateLimit(30);
  sf::Clock Clock;

  // set window coordinates
  float x0 = -2.;
  float x1 = 2.;
  float y0 = -2.;
  float y1 = 2.;
  float ratio = (y1-y0)/(x1-x0);

  //{{{ OpenGL stuff
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(x0, x1, y0, y1, 0, 1);
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0.f, 0.f, 0.f, 0.f);
  //}}}


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


    // draw f(x)
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i != samples; i++){
      float a = castFloat(i)/castFloat(samples);
      glColor3f(1., 1-a, a);
      glVertex2f(xpoints[i],ypoints[i]);
    }
    glEnd();

    // draw a spinning circle
    float time = Clock.GetElapsedTime() /* * 6.2832 */;
    circle(0.5*cos(time),0.5*sin(time),0.1);

    // cerr << 1/App.GetFrameTime() << endl;

    App.Display();
  }

  return EXIT_SUCCESS;
}
//}}}

float f(float x){
  return exp(-pow(x,2));
}

void circle(float x, float y, float r){
  glColor3f(1., .5, 0.);
  glBegin(GL_LINE_LOOP);
  for(int t = 0; t != 32; t++){
    float a = t * 0.19635;
    glVertex2f(x+r*cos(a),y+r*sin(a));
  }
  glEnd();
}

float castFloat(int i){return i;}
