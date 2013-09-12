#ifndef MOVER_HPP
#define MOVER_HPP

#include <cmath>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

static const float TWO_PI= 2*3.14159265358979f;

class Mover{
public:
   Mover(vec3 orig):position(0.0),orig_pos(orig){}
   virtual bool update(Uint32 dT, vec3& pos)=0;//does nothing
protected:
   float position; //[0..1]
   vec3 orig_pos;
};

class CircularMover: public Mover{
public:
   CircularMover(vec3 center=vec3(),float rad=0.5,Uint32 time=2500, int repetitions=-1):Mover(center),
      radius(rad),time_to_complete(time),repeats(repetitions){}
   //CircularMover(vec3 orig):Mover(orig){}
   virtual bool update(Uint32 dT, vec3& pos){
      if(repeats==0) return true;//done
      bool out = false;
      Uint32 time_to_complete_circle = 2500;//2.5s
      float pos_change = 1.0*dT/time_to_complete_circle;
      position+=pos_change;
      if(position>1.0) { if(repeats>0) --repeats; 
         position -= 1.0; out = true; }
      vec3 new_pos;
      pos.x = radius*cos(TWO_PI*position)+orig_pos.x;
      pos.y = radius*sin(TWO_PI*position)+orig_pos.y;
      pos.z = orig_pos.z;
      return out;
   }
private:
   float radius; Uint32 time_to_complete;
   int repeats;//-1 means it repeats forever
};


#endif
