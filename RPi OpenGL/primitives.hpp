#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "model.hpp"
 
// Generates models

class Primitives{
public:
   Primitives():circleId(0),linesId(0),global_shape_scale(1.0),global_pos_scale(1.0){}
   Model* circle(float cx, float cy, float rad=1.0,glm::vec3=glm::vec3(1,1,1),int line_w=1, float scale=1.0);
   Model* lines(float ox, float oy, const size_t& num_verts, const glm::vec3* vertices, const size_t& num_elems,const size_t* elements, float scale=1.0, const glm::vec3* colors=NULL);
   void set_global_shape_scale(float scale){global_shape_scale=scale;}
   float get_global_shape_scale(){return global_shape_scale;}
   void set_global_pos_scale(float scale){global_pos_scale=scale;}
   float get_global_pos_scale(){return global_pos_scale;}
private:
   int circleId;
   int linesId;
   float global_shape_scale, global_pos_scale;
};

#endif
