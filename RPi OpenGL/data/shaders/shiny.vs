attribute vec4 pos;
attribute vec3 col;
attribute vec3 norm;

uniform vec3 origin;
uniform mat4 mat_modelview;
uniform mat4 mat_projection;

uniform float audio;

varying vec3 position;
varying vec3 color;
varying vec3 transf_normal;

void main()
{
    vec4 cam_pos = mat_modelview * (pos*(1.0+audio));
    gl_Position = mat_projection * cam_pos;

    position = cam_pos.xyz;
    color = col*(clamp(audio+0.5,0.0,1.0));
    transf_normal = mat3(mat_modelview) * norm;
}
