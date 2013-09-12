#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <cinttypes>
#include <string>
#include <vector>
#include <vector>
#include <bcm_host.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <SDL.h>
#include "shader.hpp"
#include "model.hpp"
#include <unordered_map>
#include "mover.hpp"

typedef std::vector<Model*> model_list;
typedef std::unordered_map< std::string, model_list > model_map;

class Graphics
{
public:
    Graphics();
    ~Graphics();

    // Calculates normals from given positions and loads both into GPU memory.
    // Around a grid of size res_u*res_v, positions must have a frame of
    // sentinel vertices for the calculation of normals (therefore the positions
    // array has to have (res_u+2)*(res_v+2) entries).
    void load_model(const char* name,const glm::vec3* positions, const glm::vec3* colors, int res_u, int res_v);

    void render(Uint32 dT);
    void rotate_cam(float dphi, float dtheta, float droll);
    void move_cam(float dz) { cam_pos_z = glm::clamp(cam_pos_z + dz, 0.f, 100.f); }
    void reload_data() { load_shaders(); }
    void get_screen_size(int& w, int& h) const { w = sdl_screen->w;  h = sdl_screen->h; }
    void set_vsync(bool vsync) { this->vsync = vsync;  eglSwapInterval(egl_display, vsync); }
    bool get_vsync() const { return vsync; }
    void set_culling(bool culling) { if((this->culling = culling)) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE); }
    bool get_culling() const { return culling; }
    void set_wire_mode(bool wire_mode) { this->wire_mode = wire_mode; }
    bool get_wire_mode() const { return wire_mode; }
    void set_audio_data( float );
    Model* add_model(Model*);
    void draw_mandelbrot_to_texture(float scale);
    void set_mouse(int x, int y){mouse_x=x;mouse_y=y;}
    void create_new_model_group(const string& group_name){model_groups[group_name]=model_list(); curr_model_group=group_name;}
    int mouse_x,mouse_y;
    void setModelTimer(Uint32 time){modelTimer=time;}
    glm::vec3 cam_pos;
private:
    float audio_data; //[0..1]
    model_list models;
    model_map model_groups;
    string curr_model_group;

    Uint32 timer;
    Uint32 modelTimer, nextModelTime;

    void init_egl_context();
    void init_gl();
    void load_shaders();

    static GLuint compile_shader(GLenum type, const std::string& filename);
    static GLuint link_program(const std::vector<GLuint>& shaders);

    SDL_Surface *sdl_screen;
    uint32_t screen_w, screen_h;
    uint32_t cx,cy;
    EGLDisplay egl_display;
    EGLSurface egl_surface;
    bool vsync, culling, wire_mode;

    Shader* simple_shader;
    Shader* shiny_shader;
    Shader* julia_shader;
    Shader* mandelbrot_shader;
    GLuint buf;//for mandelbrot vert storage
    GLuint tex; //tex for mandel
    GLuint tex_fb;

    glm::quat cam_orient;
    float cam_pos_z;

    //Mover* mover;
    //glm::vec3 mover_pos;
};

#endif  // GRAPHICS_HPP
