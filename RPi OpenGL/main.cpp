#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <unistd.h>
#include <glm/glm.hpp>
#include <SDL.h>
#include <bcm_host.h>
#include "graphics.hpp"
#include "evaluator.hpp"
#include "exceptions.hpp"
#include "audioin.hpp"
#include "util.hpp"
#include <signal.h>
#include "primitives.hpp"
#include "mover.hpp"

using namespace std;
using namespace glm;

static const int res_u_def = 64;
static const int res_v_def = 64;

void handle_sdl_error(const char *fname)
{
    cerr << "ERROR: " << fname << ": " << SDL_GetError() << "\n";
    SDL_Quit();
    exit(1);
}



// Calculates u,v resolution and all the positions from cmd args and loads the graphics object with them.
void gen_model(int argc, char **argv, Graphics& gfx);
void gen_shapes(const string&, Graphics&);
void config(const string&,Graphics&,AudioIn&,CircularMover&);

void interrupt_handler(int s){
   printf("Caught signal %d. Exiting.\n",s);
   exit(1);
}

int main(int argc, char **argv)
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = interrupt_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
 

    bcm_host_init();

    // Initialize SDL:
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
        handle_sdl_error("SDL_Init");

    atexit(SDL_Quit);

    try
    {
       Graphics gfx;
       AudioIn audio = AudioIn();
       Smooth<float> sig_smooth(0.0f,0.0f,1.0f);
       CircularMover cam_phi_mover;
          //(cam_rotation,mover_radius,mover_time,mover_repeats);
       config("data/config.txt",gfx,audio,cam_phi_mover);
       //audio.start(capture_rate_ms); //catch audio at this interval

        // Parse command-line options and generate model:
       //gen_model(argc, argv, gfx);

        bool quitting = false;

        const int framecount_interval = 100;
        int frames = 0;
        Uint32 last_time = SDL_GetTicks();
        float v_phi = 0, v_theta = 0, v_roll = 0, v_z = 0;
        const float v_damp = 0.8;
        Uint32 last_audio_time = SDL_GetTicks();
        int mousex,mousey;
        gfx.get_screen_size(mousex,mousey);
        
        vec3 cam_rotation;
        //gfx.rotate_cam(v_phi, v_theta, v_roll);
        mousex/=2; mousey/=2;
        while(!quitting)
        {
            Uint32 this_audio_time = SDL_GetTicks();
            Uint32 delta_time = this_audio_time-last_audio_time;
            last_audio_time = this_audio_time;
            audio.update(delta_time);
            //bool is_not_done = cam_phi_mover.update(delta_time,cam_rotation);
            //gfx.rotate_cam(cam_rotation.x,0,0);//cam_rotation.y,cam_rotation.z);

            gfx.set_audio_data(
                  //sig_smooth.update(
                     audio.get_average_level()
                  //   ,delta_time)
                  ); 

            gfx.render(delta_time);

            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                case SDL_MOUSEMOTION:
                    // Keep mouse centered:
                    {
                        int w, h;
                        gfx.get_screen_size(w, h);
                        mousex = clamp(mousex+ event.motion.xrel,0,w);
                        mousey = clamp(mousey + event.motion.yrel,0,h);
                        gfx.set_mouse(mousex,mousey);
                        
                        if(event.motion.x != w/2 || event.motion.y != h/2)
                        {
                            SDL_WarpMouse(w/2, h/2);
                        }
                        else break;
                    }

                    // Camera rotation:
                    if(event.motion.state & SDL_BUTTON(1))
                    {
                        v_phi += 0.05 * event.motion.xrel;
                        v_theta += 0.05 * event.motion.yrel;
                    }

                    if(event.motion.state & SDL_BUTTON(3))
                    {
                        v_roll += 0.05 * event.motion.xrel;
                    }

                    // Camera movement:
                    if(event.motion.state & SDL_BUTTON(2))
                    {
                        v_z += 0.002 * event.motion.yrel;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if(event.button.button == SDL_BUTTON_WHEELDOWN)
                    {
                        v_z += 0.1;
                    }
                    else if(event.button.button == SDL_BUTTON_WHEELUP)
                    {
                        v_z -= 0.1;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                    case SDLK_r:
                        gfx.reload_data();
                        break;

                    case SDLK_F1:
                        cout << "\nUsage:\n"
                             << "  F1:               Display this help.\n"
                             << "  F2:               Toggle VSync.\n"
                             << "  F3:               Toggle backface culling.\n"
                             << "  F4:               Toggle wireframe rendering.\n"
                             << "  LMB / Arrow keys: Rotate camera.\n"
                             << "  RMB:              Roll camera.\n"
                             << "  MMB / Mouse wheel / Page keys:\n"
                             << "                    Move camera.\n"
                             << "\n";
                        break;

                    case SDLK_F2:
                        {
                            const bool new_vsync = !gfx.get_vsync();
                            gfx.set_vsync(new_vsync);
                            cout << "VSync turned " << (new_vsync ? "on" : "off") << ".\n";
                        }
                        break;

                    case SDLK_F3:
                        {
                            const bool new_cull = !gfx.get_culling();
                            gfx.set_culling(new_cull);
                            cout << "Culling turned " << (new_cull ? "on" : "off") << ".\n";
                        }
                        break;

                    case SDLK_F4:
                        {
                            const bool new_wire = !gfx.get_wire_mode();
                            gfx.set_wire_mode(new_wire);
                            cout << "Wireframe turned " << (new_wire ? "on" : "off") << ".\n";
                        }
                        break;

                    case SDLK_ESCAPE:
                        quitting = true;
                        break;

                    default:
                        break;
                    }
                    break;

                case SDL_QUIT:
                    quitting = true;
                    break;

                default:
                    break;
                }
            }

            const Uint8 *keystate = SDL_GetKeyState(NULL);
            // Handle camera keys:
            if(keystate[SDLK_RIGHT])    v_phi   += 0.5;
            if(keystate[SDLK_LEFT])     v_phi   -= 0.5;
            if(keystate[SDLK_DOWN])     v_theta += 0.5;
            if(keystate[SDLK_UP])       v_theta -= 0.5;
            if(keystate[SDLK_PAGEDOWN]) v_z     += 0.02;
            if(keystate[SDLK_PAGEUP])   v_z     -= 0.02;

            // Apply camera movements:
            gfx.rotate_cam(v_phi, v_theta, v_roll);
            gfx.move_cam(v_z);

            // Dampen movements:
            v_phi *= v_damp;
            v_theta *= v_damp;
            v_roll *= v_damp;
            v_z *= v_damp;

            //SDL_Delay(50);

            // Count framerate:
            if(++frames >= framecount_interval)
            {
                const Uint32 this_time = SDL_GetTicks();
                const Uint32 dt = this_time - last_time;
                if( false )
                  cout << "* " << (1000.0 * frames / dt) << " FPS\n";
                frames = 0;
                last_time = this_time;
            }
        }
    }
    catch(const exception& e)
    {
        cerr << "ERROR: " << e.what() << "\n";
    }

    SDL_Quit();
    bcm_host_deinit();

    return 0;
}


void gen_model(int argc, char **argv, Graphics& gfx)
{
    int opt;
    extern char *optarg;

    Evaluator::varlist_t varlist;
    varlist.push_back("u");
    varlist.push_back("v");
    vector<double> vars(2);

    Evaluator::constmap_t constmap;
    constmap["pi"] = M_PI;
    constmap["e"] = M_E;

    vector<Evaluator> extra_etors;
    string x_str("2*u-1"), y_str("0"), z_str("2*v-1");
    string r_str("1"), g_str("1"), b_str("1");

    int res_u = res_u_def;
    int res_v = res_u_def;

    glm::vec3 *positions = NULL;
    glm::vec3 *colors    = NULL;

    //glm::vec3 max(0,0,0);
    //glm::vec3 min(1000,1000,1000);

    try 
    {
        while((opt = getopt(argc, argv, "he:u:v:x:y:z:r:g:b:")) != -1)
        {
            switch(opt)
            {
            case 'h':
               // print_help(argv[0]);
                //exit(0);
                break;

            case 'e':
                {
                    assert(strlen(optarg) >= 3);
                    string varname;
                    varname += optarg[0];
                    varlist.push_back(varname);
                    extra_etors.push_back(Evaluator(optarg + 2, varlist, constmap));
                    vars.push_back(0.0);
                }
                break;

            case 'u':
                res_u = atoi(optarg);
                break;

            case 'v':
                res_v = atoi(optarg);
                break;

            case 'x':
                x_str = optarg;
                break;

            case 'y':
                y_str = optarg;
                break;

            case 'z':
                z_str = optarg;
                break;

            case 'r':
                r_str = optarg;
                break;

            case 'g':
                g_str = optarg;
                break;

            case 'b':
                b_str = optarg;
                break;
            }
        }

        assert(res_u > 1 && res_v > 1 && res_u * res_v <= 256 * 256);

        // Position evaluators:
        Evaluator x_eval(x_str, varlist, constmap),
                  y_eval(y_str, varlist, constmap),
                  z_eval(z_str, varlist, constmap);
        // Color evaluators:
        Evaluator r_eval(r_str, varlist, constmap),
                  g_eval(g_str, varlist, constmap),
                  b_eval(b_str, varlist, constmap);

        // Calculate vertex positions and colors:
        positions = new glm::vec3[(res_u + 2) * (res_v + 2)];
        colors    = new glm::vec3[res_u * res_v];
        for(int j=-1; j<res_v+1; ++j)
        {
            vars[1] = 1.0 * j / (res_v - 1);  // v
            for(int i=-1; i<res_u+1; ++i)
            {
                vars[0] = 1.0 * i / (res_u - 1);  // u

                // Calculate auxiliary variables:
                for(size_t k=0; k<extra_etors.size(); ++k)
                {
                    vars[2 + k] = extra_etors[k].evaluate(vars);
                }

                const int pos_idx = (i+1) + (res_u+2) * (j+1);

                // Evaluate position:
                positions[pos_idx] = glm::vec3(x_eval.evaluate(vars),
                                               y_eval.evaluate(vars),
                                               z_eval.evaluate(vars));
                /*if(positions[pos_idx].x>max.x)max.x=positions[pos_idx].x;
                if(positions[pos_idx].y>max.y)max.y=positions[pos_idx].y;
                if(positions[pos_idx].z>max.z)max.z=positions[pos_idx].z;
                if(positions[pos_idx].x<min.x)min.x=positions[pos_idx].x;
                if(positions[pos_idx].y<min.y)min.y=positions[pos_idx].y;
                if(positions[pos_idx].z<min.z)min.z=positions[pos_idx].z;*/

                // Evaluate colors:
                if(i >= 0 && i < res_u && j >= 0 && j < res_v)
                {
                    colors[i + res_u * j] = glm::vec3(r_eval.evaluate(vars),
                                                      g_eval.evaluate(vars),
                                                      b_eval.evaluate(vars));
                }
            }
        }
    }
    catch(const string& e)
    {
        cout << "PARSE ERROR: " << e << "\n";
        exit(1);
    }

    gfx.load_model("pizza_party",positions, colors, res_u, res_v);
    //cout<<"MAX POS: ["<<max.x<<", "<<max.y<<", "<<max.z<<"]"<<endl;
    //cout<<"MIN POS: ["<<min.x<<", "<<min.y<<", "<<min.z<<"]"<<endl;


    delete[] positions;
    delete[] colors;
}

template <class T>
T str_to_num(string& s){
   T out;
   stringstream convert(s);
   if(!(convert>>out))
      out=0;
   return out;
}

Model* gen_circle(vector<string>& tokens, Primitives& p, Graphics& gfx){
  unsigned int i=1;
  float cx = str_to_num<float>(tokens[i++]);
  float cy = str_to_num<float>(tokens[i++]);
  float rad = str_to_num<float>(tokens[i++]);
  vec3 color(1,1,1);
  int line_w=2;
  float scale=1.0;
  if(tokens.size()>i){
      color.x =  str_to_num<float>(tokens[i++]);
      color.y =  str_to_num<float>(tokens[i++]);
      color.z =  str_to_num<float>(tokens[i++]);
      if(tokens.size()>i){
         scale = str_to_num<float>(tokens[i++]);
         if(tokens.size()>i)
            line_w = str_to_num<int>(tokens[i++]);
      }
  }
  return gfx.add_model( p.circle(cx,cy,rad,color,line_w,scale) );
}

Model* gen_lines(vector<string>& first, vector<vector<string> >& verts, vector<string>& elements, Primitives& p, Graphics& gfx){
   float scale = 1.0f;
   float ox = str_to_num<float>(first[1]);
   float oy = str_to_num<float>(first[2]);
   int line_w = 2;
   if(first.size()>4) scale = str_to_num<float>(first[4]);
   if(first.size()>5) line_w = str_to_num<int>(first[5]);
   size_t num_verts = verts.size();
   vec3* verts_f=new vec3[num_verts];
   vec3* colors = new vec3[num_verts];
   for(size_t i=0; i < num_verts; ++i){
      verts_f[i] = vec3(str_to_num<float>(verts[i][1]),
                        str_to_num<float>(verts[i][2]), 
                        0.0 );
      if(verts[i].size()>3){
         colors[i]=vec3( str_to_num<float>(verts[i][3]),
                         str_to_num<float>(verts[i][4]),
                         str_to_num<float>(verts[i][5]));
      }else colors[i]=vec3(1,1,1);//white default vert color
   }
   size_t num_elems = elements.size()-(elements.size()%2); //ensure pairs
   size_t* elems = new size_t[num_elems];
   for(size_t i=0; i<num_elems; ++i){
      elems[i]=(str_to_num<int>(elements[i]));
   }
   Model* m = gfx.add_model( p.lines(ox,oy,num_verts,verts_f,num_elems,elems,scale,colors));
   //m->set_mover(new CircularMover(m->get_position()));
   m->set_line_width(line_w);
   delete[] verts_f;
   delete[] colors;
   delete[] elems;
   return m;
}

vector<string> get_tokens(string& line){
   vector<string> tokens;
   istringstream iss(line);
   copy(istream_iterator<string>(iss),
        istream_iterator<string>(),
        back_inserter<vector<string> >(tokens));
   return tokens;
}

void gen_shapes(const string& file, Graphics& gfx){
   ifstream fin;
   fin.open(file.c_str());
   if( !fin.good() ){
      cout<<"Error opening "<<file<<" for reading in shapes"<<endl;
      exit(1);
   }
   gfx.create_new_model_group(file); //add these shapes to a new model group
   Primitives prim_maker;
   string line="";
   int line_nbr=0;
   bool next_model_static=false;
   Model* m=NULL;
   try{
   while(getline(fin,line)){
      m=NULL;
      ++line_nbr;
      vector<string> tokens = get_tokens(line);
      if(tokens.size()==0)continue;
      if(tokens[0].compare("#")==0)continue;
      if(tokens[0].compare("global_pos_scale")==0)
         prim_maker.set_global_pos_scale(str_to_num<float>(tokens[1]));
      else if(tokens[0].compare("global_shape_scale")==0)
         prim_maker.set_global_shape_scale(str_to_num<float>(tokens[1]));
      else if(tokens[0].compare("circle")==0)
         m=gen_circle(tokens,prim_maker,gfx);
      else if(tokens[0].compare("lines")==0){
         int num_verts = str_to_num<int>(tokens[3]);
         vector<vector<string> > verts;
         for(int i=0;i<num_verts;++i){
            getline(fin,line);
            verts.push_back(get_tokens(line));
         }
         getline(fin,line);
         vector<string> elements = get_tokens(line);
         //cout<<"in "<<file<<" line elements length="<<elements.size()<<endl;

         m=gen_lines(tokens,verts,elements,prim_maker,gfx);
      }else if(tokens[0].compare("static")==0){
         next_model_static=true;
      }
      if(m!=NULL&&next_model_static){
         m->set_static(true);
         next_model_static=false;
      }
   }
   }catch(exception& e){
      cout << "Error parsing shapes at "<<file<<":"<<line_nbr<<endl;
      exit(1);
   }
}


void config(const string& config_file,Graphics& gfx,AudioIn& audio,CircularMover& mover){
   ifstream fin;
   fin.open(config_file.c_str());
   if( !fin.good() ){
      cout<<"Error opening "<<config_file<<" for reading in config"<<endl;
      exit(1);
   }
   string line="";
   int line_nbr=0;
   int cap_rate = 50;//default
   try{
      while(getline(fin,line)){
         ++line_nbr;
         vector<string> tokens = get_tokens(line);
         if(tokens.size()==0)continue;
         if(tokens[0].compare("#")==0)continue;
         if(tokens[0].compare("shape_file")==0){
            gen_shapes(tokens[1],gfx);
         }else if(tokens[0].compare("capture_rate")==0){
            cap_rate = str_to_num<int>(tokens[1]);
         }else if(tokens[0].compare("circular_mover")==0){
            float rad = 3.14;
            Uint32 time = 5000;
            int repeats = -1;
            rad = str_to_num<float>(tokens[1]);
            if(tokens.size()>2)time = str_to_num<Uint32>(tokens[2]);
            if(tokens.size()>3)repeats = str_to_num<int>(tokens[3]);
            mover = CircularMover(vec3(),rad,time,repeats);
         } 
      }
   }catch(exception& e){
      cout<<"Error parsing config file at "<<config_file<<":"<<line_nbr<<endl;
      exit(1);
   }
   audio.start(cap_rate);
}

