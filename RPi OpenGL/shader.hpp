#ifndef SHADER_HPP
#define SHADER_HPP

#include <cinttypes>
#include <string>
#include <vector>
#include <bcm_host.h>
#include <glm/glm.hpp>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <SDL.h>

class Shader
{
public:
	Shader(){} //intentionally left empty
    Shader(const std::string& vert,const std::string& frag);
    ~Shader(){}
	Shader& operator=(const Shader& other);
   void enable();
   void disable();
	GLuint get_program(){return prog;}
	GLuint get_projmat(){return uni_projmat;}
	GLuint get_modelmat(){return uni_modelmat;}
	GLuint get_attr_pos(){return attr_pos;}
	GLuint get_attr_col(){return attr_col;}
	GLuint get_attr_norm(){return attr_norm;}
   GLuint get_audio(){return attr_audio;}
   GLuint get_uni_origin(){return uni_origin;};
   GLuint uni_scale, uni_offset, uni_tex, uni_centre, uni_color;
private:
    void load_shader(const std::string& vert,const std::string& frag);
    static GLuint compile_shader(GLenum type, const std::string& filename);
    static GLuint link_program(const std::vector<GLuint>& shaders);
	
    GLuint prog;
    GLuint attr_pos, attr_col, attr_norm;
    GLuint uni_modelmat, uni_projmat;
    GLuint uni_origin;

    GLuint attr_audio;
};

#endif  // SHADER_HPP
