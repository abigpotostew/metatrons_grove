#ifndef MODEL_HPP
#define MODEL_HPP


#include "shader.hpp"
#include <glm/glm.hpp>
#include <string>
//#include "mover.hpp"

using namespace std;

enum ModelType { COMPLEX, SIMPLE };

class Model {
public:
   friend class Primitives;
	string name;
	Model(const char* _name) : name(_name),
      pos(0,0,0),
      //model_view(1),//identity matrix
      draw_mode(GL_TRIANGLE_STRIP),
      line_width(1.0), type(COMPLEX),
      static_model(false) //,mover(NULL)
   {}
   //~Model(){ if(mover!=NULL) delete mover; }
	void render(glm::mat4 cam_view,Shader* shader);
	void load_complex(const glm::vec3* positions, const glm::vec3* colors, int res_u, int res_v);
   void set_draw_mode(GLenum mode){draw_mode=mode;}
   void set_line_width(GLfloat l_width){line_width=l_width;}
	void bind_indices_array(uint16_t* elems,const size_t num_elements);
	void bind_vertices_array(float* verts, const size_t n_verts);
   ModelType get_type(){return type;}
   void set_pos(glm::vec3);
   glm::vec3 get_position(){return pos;}
   void set_static(bool has_static){static_model=has_static;}
   bool is_static(){return static_model;}
   //void update(Uint32 dT){if(mover!=NULL)mover->update(dT,pos);}
   //void set_mover(Mover* new_mover){mover=new_mover;}
protected:
	GLuint vao, vbo_model, ibo_model, ibo_model_wire;
   int res_u, res_v;
   size_t n_verts;
	size_t n_draw_elements, n_draw_wire_elements;
   glm::vec3 pos;
   //glm::mat4 model_view;
   GLenum draw_mode;
   GLfloat line_width;
   ModelType type;
private:
   bool static_model;
   //Mover* mover;
};



#endif
