attribute vec4 pos;
varying vec2 tcoord;

void main(void){
   vec4 vertex = pos;
   gl_Position = vertex;
   tcoord = vertex.xy*0.5+0.5;
}
