import ddf.minim.analysis.*;
import ddf.minim.*;

boolean DEBUG = false;
PFont font;

//Audio
Minim minim;  
AudioInput in;

//CONSTANTS:
final int Width = 700, Height=700;
final float diameter = Height*0.20f,
            radius = diameter/2,
            hW = Width/2, hH=Height/2;
//Every line to connect metatron's cube without any overlap.
final int[] lines = { 
  //TOP LEFT
  2,12, 12,10, 12,11, 12,1, 2,0, 2,9, 1,0, 11,0, 11,9, 2,11, 9,0, 
  10,8, 10,9, 11,10, 12,9, 10,5, 1,9, 11,7, 9,7, 10,7, 9,6, 9,8,
  0,7, 7,8, 11,8, 10,1, 12,7, 
  //RIGHT HALF
  11,1, 2,4, 1,3, 2,3, 12,3, 0,3, 4,3, 0,5, 3,5, 3,6, 4,5, 2,5,
  3,7, 9,5, 1,5, 7,5, 5,8, 11,4, 7,6, 8,3, 6,1, 11,3, 8,6, 6,4, 
  //BOTTOM RIGHT
  4,1, 5,6, 7,4, 8,5
};
final int numLines = lines.length/2;
final float lineColorSpeed = 10f;
final int HUERANGE = 100;

//VARIABLES:
float minDiameter = diameter*.5f, //for the bouncing circles
      maxDiameter = diameter*1.5f;//^
PVector[] nodes;
Smooth bounce; //TODO: Fix up Smooth class to work more smoothly :D
float deltaTime; //set to 1/frameRate each tic
float lineColorOffset=0f;

//TODO: monitor external audio volume to adjust for crazy sounds levels.
float audioMin=100000,audioMax=-100000;


void setup(){
  size(Width,Height);
  //size(displayWidth,displayHeight);
  colorMode(HSB, HUERANGE);
  
  font = loadFont("AdobeMyungjoStd-Medium-15.vlw");
  textFont(font, 15);

  //noSmooth();//faster?
  
  //Audio setup:
  minim = new Minim(this);
  in = minim.getLineIn();
  
  strokeWeight(2);
  noFill();
  
  //Map of node indices
  /*
        12
10      11      2
    9       1
        0
    7       3
8       5       4
        6
  */
  nodes = new PVector[13];
  nodes[0] = new PVector(0,0);//center node is at (0,0)
  for(int i = 0; i < 6; ++i){
    int x = (int)(Math.cos(2*PI/6*(i-0.5f))*diameter);
    int y = (int)(Math.sin(2*PI/6*(i-0.5f))*diameter);
    nodes[i*2+1] = new PVector(x,y);
    nodes[i*2+2] = new PVector(x*2,y*2);
  }
  
  bounce = new Smooth(diameter,0.002f,0.0002f);
}

void draw(){
  deltaTime = 1f/frameRate;
  background(0);
  pushMatrix();
  translate(hW,hH);
  //rotate(millis()/10000f);
  float d = 0f;
  for(int i = 0; i < in.bufferSize() - 1; i++){
    d+=in.left.get(i);
  }
  updateAudioRange(d);
  //updateDiameterRange();
  d=clamp(bounce.update(d*maxDiameter+diameter),minDiameter,maxDiameter);
  color circleColor = color((d-minDiameter)/(maxDiameter-minDiameter)*HUERANGE,80,90);
  //int lineCt=0;
  stroke(circleColor);
  for(int i=0;i<nodes.length;++i){
    PVector p = nodes[i];
    ellipse(p.x,p.y,d,d);
    if(DEBUG){
      fill(HUERANGE);
      text(""+i, p.x,p.y);
      noFill();
    }
  }
  float colorStep=0.02f*numLines;
  float colorDelta=lineColorOffset*HUERANGE;
  for(int i=0;i<lines.length;i+=2){
    stroke((colorDelta)%HUERANGE,90,90);
    line(nodes[lines[i]].x,nodes[lines[i]].y,
      nodes[lines[i+1]].x,nodes[lines[i+1]].y);
    colorDelta+=colorStep;
  }
  lineColorOffset=(lineColorOffset+deltaTime/lineColorSpeed);
  if(lineColorOffset>1f)lineColorOffset-=1f;
  popMatrix();
}

boolean sketchFullScreen(){
  return false;
}

//Smoothly steer a value to a destination.
//Used for the bouncy audio-reactive circles
//TODO: Make it more smooth
class Smooth{
  float value,accel,speed; //speed is in s, and is equivalent to accel
  private float pos, vel;
  public Smooth(float _value,float _speed, float _accel){
    value=_value;
    speed=_speed;
    pos=0f;
    vel=0f;
    accel = _accel;
  }
  public float update(float dest){
    vel += accel*deltaTime*(dest<pos?-1f:1f);//clamp(accel*deltaTime*(dest<pos?-1f:1f),-speed,speed);
    pos += vel;
    if(pos<0f||pos>1f)vel=0;
    value = lerp(value,dest,pos);
    return value;
  }
  public float get(){return value;}
}

void updateAudioRange(float current){
  if(current<audioMin)audioMin=current;
  if(current>audioMax)audioMax=current;
}

void updateDiameterRange(){
  minDiameter=diameter*.5f;
  maxDiameter=diameter*1.5f;
}

float clamp(float val,float Min,float Max){
  return (val<Min) ? Min : ((val>Max) ? Max : val);
}
