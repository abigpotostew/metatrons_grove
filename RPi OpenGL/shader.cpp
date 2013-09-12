#include <cassert>
#include <cinttypes>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include "exceptions.hpp"
using namespace std;
using namespace glm;

Shader::Shader(const std::string& vert,const std::string& frag)
{
    load_shader(vert,frag);
}

Shader& Shader::operator=(const Shader& other){
    prog = other.prog;
    attr_pos = other.attr_pos;
	attr_col = other.attr_col;
	attr_norm = other.attr_norm;
   attr_audio = other.attr_audio;
    uni_modelmat = other.uni_modelmat;
	uni_projmat = other.uni_projmat;
	return *this;
}

void Shader::enable(){
   glUseProgram(get_program());
   //glUniformMatrix set matrix in model
   glEnableVertexAttribArray(get_attr_pos());
   glEnableVertexAttribArray(get_attr_col());
   //if(get_attr_norm()!=-1) 
   glEnableVertexAttribArray(get_attr_norm());
}

void Shader::load_shader(const std::string& vert,const std::string& frag)
{
    // If the shader program is already loaded, delete it first:
    if(prog != 0) glDeleteProgram(prog);

    // Simple shader program:
    // Compile shaders:
    vector<GLuint> shaders;
    shaders.push_back(compile_shader(GL_VERTEX_SHADER, vert));
    shaders.push_back(compile_shader(GL_FRAGMENT_SHADER, frag));

    // Link program:
    prog = link_program(shaders);

    // Delete shaders again:
    for_each(shaders.begin(), shaders.end(), glDeleteShader);

    // Get locations:
    attr_pos  = glGetAttribLocation(prog, "pos");
    attr_col  = glGetAttribLocation(prog, "col"); //return -1 if it doesn't exist
	attr_norm  = glGetAttribLocation(prog, "norm");
    uni_modelmat = glGetUniformLocation(prog, "mat_modelview");
    uni_projmat  = glGetUniformLocation(prog, "mat_projection");

    attr_audio = glGetUniformLocation(prog, "audio");

    uni_origin = glGetUniformLocation(prog, "origin");

    //fractal stuff
    uni_scale = glGetUniformLocation(prog, "scale");
    uni_offset = glGetUniformLocation(prog, "offset");
    uni_tex = glGetUniformLocation(prog, "tex");
    uni_centre = glGetUniformLocation(prog, "centre");
    uni_color = glGetUniformLocation(prog, "u_color");
}



GLuint Shader::compile_shader(GLenum type, const std::string& filename)
{
    char *shader_text = NULL;
    int shader_length = 0;

    // Read file contents:
    //   Open file:
    ifstream file_stream(filename.c_str());

    //   Get file length:
    file_stream.seekg(0, ios::end);
    shader_length = file_stream.tellg();
    file_stream.seekg(0, ios::beg);

    //   Read into buffer:
    shader_text = new char[shader_length];
    file_stream.read(shader_text, shader_length);

    //    Close file:
    file_stream.close();


    // Create OpenGL shader:
    const GLuint handle = glCreateShader(type);
    const char *const_shader_text = shader_text;
    glShaderSource(handle, 1, &const_shader_text, &shader_length);
    delete[] shader_text;
    glCompileShader(handle);

    // Check for errors:
    GLint status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
        // Get info log:
        GLint log_length;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
        GLchar *log_text = new GLchar[log_length];
        glGetShaderInfoLog(handle, log_length, NULL, log_text);

        // Print log:
        cerr << "Compilation of \"" << filename << "\" failed:\n" << log_text << "\n";
        delete[] log_text;

        throw GLException("shader compilation failed");
    }

    return handle;
}

GLuint Shader::link_program(const vector<GLuint>& shaders)
{
    typedef vector<GLuint>::const_iterator IT;

    // Create OpenGL program:
    const GLuint handle = glCreateProgram();

    // Attach the shaders:
    for(IT it = shaders.begin(); it != shaders.end(); ++it)
        glAttachShader(handle, *it);

    // Link:
    glLinkProgram(handle);

    // Detach the shaders:
    for(IT it = shaders.begin(); it != shaders.end(); ++it)
        glDetachShader(handle, *it);

    // Check for errors:
    GLint status;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        // Get info log:
        GLint log_length;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
        GLchar *log_text = new GLchar[log_length];
        glGetProgramInfoLog(handle, log_length, NULL, log_text);

        // Print log:
        cerr << "Linking of shader program failed:\n" << log_text << "\n";
        delete[] log_text;

        throw GLException("program linking failed");
    }

    return handle;
}
