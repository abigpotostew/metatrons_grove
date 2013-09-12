#include <cassert>
#include <cinttypes>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "graphics.hpp"
#include "exceptions.hpp"
#include "shader.hpp"
#include "model.hpp"

using namespace std;
using namespace glm;

#define check() assert(glGetError()==0)

Graphics::Graphics()
  : curr_model_group(""), timer(0), modelTimer(10000), nextModelTime(5000),
   vsync(true), culling(false), wire_mode(false),
    cam_orient(quat(vec3(0.f, 0.f, 0.f))),
    cam_pos_z(7)
{
    sdl_screen = SDL_SetVideoMode(0, 0, 0, SDL_SWSURFACE | SDL_FULLSCREEN);
    if(!sdl_screen) throw SDLException("SDL_SetVideoMode");

    init_egl_context();
    init_gl();
    load_shaders();
    
    curr_model_group = "";
    timer=0;
    modelTimer = 10000;
    srand(time(NULL));

}

Graphics::~Graphics()
{
   for(auto itor = models.begin(); itor!=models.end();++itor){
      delete *itor;;
   }
   models.clear();
	delete simple_shader;
	delete shiny_shader;
}

void Graphics::render(Uint32 dT)
{
    timer += dT;
    //cout<<dT<<endl;
    if(timer>nextModelTime){
      //cout<<"timer="<<timer<<endl;
      timer = 1;
      Uint32 half_timer = modelTimer/2;
      nextModelTime = modelTimer + (rand()%half_timer-rand()%half_timer);
      auto i = model_groups.find(curr_model_group);
      ++i;
      if(i == model_groups.end()) i= model_groups.begin();
      curr_model_group = i->first;
      //cout<<"rendering "<<curr_model_group<<"for "<<nextModelTime<<"ms."<<endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   

  /* glBindBuffer(GL_ARRAY_BUFFER,buf);
   glUseProgram(julia_shader->get_program());
   glBindTexture(GL_TEXTURE_2D,tex);
   glUniform4f(julia_shader->uni_color,0.5,0.5,0.8,1.0);
   glUniform2f(julia_shader->uni_scale, 0.003,0.003);
   glUniform2f(julia_shader->uni_offset,mouse_x,mouse_y);
   glUniform2f(julia_shader->uni_centre,cx,cy);
   glUniform1i(julia_shader->uni_tex,0);

   //glEnableVertexAttribArray(julia_shader->get_attr_pos());
   //glVertexAttribPointer(julia_shader->get_attr_pos(), 4, GL_FLOAT,0,16,0);
   
   glDrawArrays(GL_TRIANGLE_FAN,0,4);
   check();
   //glBindBuffer(GL_ARRAY_BUFFER,0);//unbind the verts buffer
*/

   Shader* curr_shader = NULL;
   Model* m = NULL;
   model_list models = model_groups[curr_model_group];
    for(auto itor = models.begin(); itor!=models.end();++itor){
       m = *itor;
       //cout<<"rendering "<<m->name<<endl;
       //m->update(dT);
       //m->set_static(true);
       if( m->get_type() == SIMPLE ) curr_shader = simple_shader;
       else curr_shader= shiny_shader;
       curr_shader->enable();
       glm::mat4 modelview(1);
       modelview = translate(modelview, (m->get_position()) + vec3(cam_pos.x,cam_pos.y,cam_pos.z-cam_pos_z));
       modelview = modelview * mat4_cast(cam_orient);
       glUniformMatrix4fv(curr_shader->get_modelmat(),1,GL_FALSE,value_ptr(modelview));
       float audio = 0.0;
       if(!m->is_static())
          audio=this->audio_data;
       glUniform1f(curr_shader->get_audio(),audio);
       glUniform3fv(curr_shader->get_uni_origin(),1,value_ptr(m->get_position()));
       (*itor)->render(modelview, curr_shader); 
    } 

      glBindBuffer(GL_ARRAY_BUFFER,0);
  // glFlush();
   //glFinish();
   //check();

    eglSwapBuffers(egl_display, egl_surface);
}

void Graphics::init_gl()
{
    glViewport(0, 0, screen_w, screen_h);

    glClearColor(0.0, 0.0,0.0, 1.0);

    //glClearDepthf(1);
    //glEnable(GL_DEPTH_TEST);
    //glDepthRangef(0, 1);
    //glDepthFunc(GL_LESS);

    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void Graphics::draw_mandelbrot_to_texture(float scale){

   glBindFramebuffer(GL_FRAMEBUFFER,tex_fb);
   check();
   glBindBuffer(GL_ARRAY_BUFFER,buf);
   //glEnableVertexAttribArray(julia_shader->get_attr_pos());
   //check();
   //glVertexAttribPointer(julia_shader->get_attr_pos(), 4, GL_FLOAT,GL_FALSE, 0,0); 
   //check();

   glUseProgram(mandelbrot_shader->get_program());
   check();


   glUniform2f(mandelbrot_shader->uni_scale,scale,scale);
   glUniform2f(mandelbrot_shader->uni_centre,cx,cy);
   check();
   glDrawArrays(GL_TRIANGLE_FAN,0,4);
   check();

   glFlush();
   glFinish();
   check();
}

void Graphics::load_shaders()
{
    simple_shader = new Shader("data/shaders/simple.vs","data/shaders/simple.fs");
    glGetError();
    shiny_shader = new Shader("data/shaders/shiny.vs","data/shaders/shiny.fs");	
    //glGetError();
    //julia_shader = new Shader("data/shaders/fractal.vs","data/shaders/julia.fs"); 
    //glGetError();
	
    //mandelbrot_shader = new Shader("data/shaders/fractal.vs","data/shaders/mandelbrot.fs"); 
    //glGetError();

    //TODO:loop over all shaders and do this function
    // Set constant uniforms:
    float ratio = 1.f * screen_w / screen_h;
    mat4 projection = perspective(45.f, ratio, .1f, 100.f);
    glUseProgram(simple_shader->get_program());
    glUniformMatrix4fv(simple_shader->get_projmat(), 1, GL_FALSE, value_ptr(projection));
    glUseProgram(shiny_shader->get_program());
    glUniformMatrix4fv(shiny_shader->get_projmat(), 1, GL_FALSE, value_ptr(projection));
    glUseProgram(0);


/*
    glGenBuffers(1, &buf);
    check();
    glGenTextures(1,&tex);
    check();
    glBindTexture(GL_TEXTURE_2D,tex);
    check();
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,screen_w,screen_h,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    check();

    glGenFramebuffers(1,&tex_fb);
    check();
    glBindFramebuffer(GL_FRAMEBUFFER,tex_fb);
    check();
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex,0);
    check();
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    //set vertex data in fractal shaders);
    static const GLfloat vertex_data[]={
      -1.0,-1.0,1.0,1.0,
      1.0,-1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,
      -1.0,1.0,1.0,1.0
    };
    cx = screen_w/2;
    cy = screen_h/2;
    
   glBindBuffer(GL_ARRAY_BUFFER,buf);
    check();
   glBufferData(GL_ARRAY_BUFFER,sizeof(vertex_data),vertex_data,GL_STATIC_DRAW);
    check();
   glVertexAttribPointer(julia_shader->get_attr_pos(), 4, GL_FLOAT, 0, 16, 0);
   glEnableVertexAttribArray(julia_shader->get_attr_pos());
   check();

   draw_mandelbrot_to_texture(0.003);*/
}

void Graphics::load_model(const char* name,const glm::vec3* positions, const glm::vec3* colors, int res_u, int res_v)
{
    Model* m=new Model(name);
    m->load_complex(positions,colors,res_u,res_v);
    models.push_back(m);

}

void Graphics::rotate_cam(float dphi, float dtheta, float droll)
{
    dphi *= M_PI / 180.f;
    dtheta *= M_PI / 180.f;
    droll *= M_PI / 180.f;

    cam_orient = normalize(cross(quat(vec3(dtheta, dphi, -droll)), cam_orient));
}
 
void Graphics::init_egl_context()
{
    // Get an EGL display connection:
    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(egl_display == EGL_NO_DISPLAY)
        throw EGLException("eglGetDisplay");
 
    // Initialize the EGL display connection:
    if(eglInitialize(egl_display, NULL, NULL) == EGL_FALSE)
        throw EGLException("eglInitialize");
 
    // Get an appropriate EGL frame buffer configuration:
    EGLConfig config;
    {
        EGLint num_config;
        static const EGLint attribute_list[] =
        {
           EGL_RED_SIZE, 8,
           EGL_GREEN_SIZE, 8,
           EGL_BLUE_SIZE, 8,
           EGL_ALPHA_SIZE, 8,
           EGL_DEPTH_SIZE, 8,
           EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
           EGL_NONE
        };
 
        if(eglChooseConfig(egl_display, attribute_list, &config, 1, &num_config) == EGL_FALSE)
            throw EGLException("eglChooseConfig");
    }
 
    // Get an appropriate EGL frame buffer configuration:
    if(eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE)
        throw EGLException("eglBindAPI");
 
    // Create an EGL rendering context:
    EGLContext context;
    {
        static const EGLint context_attributes[] = 
        {
           EGL_CONTEXT_CLIENT_VERSION, 2,
           EGL_NONE
        };
 
        context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT, context_attributes);
        if(context == EGL_NO_CONTEXT)
            throw EGLException("eglCreateContext");
    }
 
    // Create an EGL window surface:
    if(graphics_get_display_size(0 /* LCD */, &screen_w, &screen_h) < 0)
        throw EGLException("graphics_get_display_size");
 
    {
        static EGL_DISPMANX_WINDOW_T nativewindow;
 
        DISPMANX_ELEMENT_HANDLE_T dispman_element;
        DISPMANX_DISPLAY_HANDLE_T dispman_display;
        DISPMANX_UPDATE_HANDLE_T dispman_update;
        VC_RECT_T dst_rect;
        VC_RECT_T src_rect;
 
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.width = screen_w;
        dst_rect.height = screen_h;
           
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.width = screen_w << 16;
        src_rect.height = screen_h << 16;        
 
        dispman_display = vc_dispmanx_display_open(0 /* LCD */);
        dispman_update = vc_dispmanx_update_start(0);
              
        dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display, 0 /*layer*/, &dst_rect, 0 /*src*/,
          &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0 /*clamp*/, DISPMANX_NO_ROTATE /*transform*/);
           
        nativewindow.element = dispman_element;
        nativewindow.width = screen_w;
        nativewindow.height = screen_h;
        vc_dispmanx_update_submit_sync( dispman_update );
           
        egl_surface = eglCreateWindowSurface(egl_display, config, &nativewindow, NULL);
        if(egl_surface == EGL_NO_SURFACE)
            throw EGLException("eglCreateWindowSurface (check your RAM split)");
    }
 
    // Connect the context to the surface:
    if(eglMakeCurrent(egl_display, egl_surface, egl_surface, context) == EGL_FALSE)
        throw EGLException("eglMakeCurrent");
 
    // Extra EGL settings:
    if(eglSwapInterval(egl_display, vsync) == EGL_FALSE)
        throw EGLException("eglSwapInterval");
}


void Graphics::set_audio_data(float value ){
   this->audio_data = (value<0.0)?0.0 : ((value>1.0)?1.0:value);//clamp it
}

Model* Graphics::add_model(Model* m){
   models.push_back(m);
   model_groups[curr_model_group].push_back(m);
   return m;
}
