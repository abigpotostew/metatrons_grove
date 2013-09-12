attribute vec4 pos; //vert pos
attribute vec3 col;
attribute vec3 norm;

uniform vec3 origin;//origin
uniform mat4 mat_modelview;
uniform mat4 mat_projection;

uniform float audio;

varying vec3 color;
varying vec3 transf_normal;

void main()
{
    vec4 v_pos =  vec4(pos.xyz,0.0)*audio+pos; //+ vec4(origin,0.0);
    vec4 cam_pos = mat_modelview * v_pos;
    gl_Position = mat_projection * cam_pos;

    color = col*clamp(audio+0.5,0.0,1.0);
    transf_normal = norm;
}

