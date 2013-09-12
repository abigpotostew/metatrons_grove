#include <glm/glm.hpp>
#include <iostream>
#include <cmath>

#include "primitives.hpp"
#include "model.hpp"

const double PI  =3.141592653589793238462;
const double TWO_PI = 2*3.141592653589793238462;
const float  PI_F=3.14159265358979f;
const float TWO_PI_F= 2*3.14159265358979f;

using namespace std;
using namespace glm;

Model* Primitives::circle(float cx, float cy, float rad,
                          vec3 color, int line_w, float scale ){
   float shape_scale = this->global_shape_scale*scale;
   Model* m = new Model("circle"+(circleId++));
   m->type = SIMPLE;
   int precision = 100*rad*shape_scale+15;
   float* verts = new float[3*3*precision];
   for(int i = 0; i < precision; ++i){
      int idx = i*3;
      verts[idx + 0] = rad*cos(i*(TWO_PI/precision))*shape_scale;
      verts[idx + 1] = rad*sin(i*(TWO_PI/precision))*shape_scale;
      verts[idx + 2] = 0;

      verts[3*precision+idx + 0] = color.x; //colors
      verts[3*precision+idx + 1] = color.y;
      verts[3*precision+idx + 2] = color.z;

      verts[6*precision+idx + 0] = 0.0f; //norms
      verts[6*precision+idx + 1] = 0.0f;
      verts[6*precision+idx + 2] = 1.0f;
   }
   m->bind_vertices_array(verts,precision);
   delete[] verts;

   uint16_t* elems = new uint16_t[precision-1]; 
   for(int i = 0; i < precision-1; ++i){
      elems[i] = (uint16_t)i;
   }
   m->bind_indices_array(elems,precision-1);
   delete[] elems;
   m->set_draw_mode(GL_LINE_LOOP);
   m->set_line_width(line_w);
   m->set_pos(glm::vec3(cx*global_pos_scale,cy*global_pos_scale,0));
   return m;
}

Model* Primitives::lines(float ox, float oy,const size_t& num_verts, const glm::vec3* vertices, const size_t& num_elems, const size_t* elements, float scale,const vec3* colors){
   float shape_scale = this->global_shape_scale*scale;
   float line_w = 1.0f;
   Model* m = new Model("lines"+(linesId++));
   m->type = SIMPLE;
   float* verts = new float[3*3*num_verts];
   for(size_t i = 0; i < num_verts; ++i){
      int idx = i*3;
      verts[idx + 0] = vertices[i].x*shape_scale;
      verts[idx + 1] = vertices[i].y*shape_scale;
      verts[idx + 2] = vertices[i].z*shape_scale;
      
      if(colors!=NULL){
         verts[3*num_verts+idx + 0] = colors[i].x; //colors
         verts[3*num_verts+idx + 1] = colors[i].y;
         verts[3*num_verts+idx + 2] = colors[i].z;
      }else{
         verts[3*num_verts+idx + 0] = 1.0f; //colors
         verts[3*num_verts+idx + 1] = 1.0f;
         verts[3*num_verts+idx + 2] = 1.0f;
      }

      verts[6*num_verts+idx + 0] = 0.0f; //norms
      verts[6*num_verts+idx + 1] = 0.0f;
      verts[6*num_verts+idx + 2] = 1.0f;
   }
   m->bind_vertices_array(verts,num_verts);
   delete[] verts;

   uint16_t* elems = new uint16_t[num_elems]; 
   for(size_t i = 0; i < num_elems; ++i){
      elems[i] = elements[i];
   }
   m->bind_indices_array(elems,num_elems);
   delete[] elems;

   m->set_draw_mode(GL_LINES);
   m->set_line_width(line_w);
   m->set_pos(glm::vec3(ox*global_pos_scale,oy*global_pos_scale,0));
   return m;
   
}
