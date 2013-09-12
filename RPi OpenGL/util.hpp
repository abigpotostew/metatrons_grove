#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>

template <class T>
T clamp(T v, T& min, T& max){
   if ( v < min ) v = min;
   else if ( v > max ) v = max;
   return v;
}

template <class T>
class Smooth{
   public:
      Smooth(T _value, T _min, T _max):value(_value),accel(0.01f),
         decel(0.001f),vel(0.0f), min(_min), max(_max), pos((_value-_min)/(_max-_min)){ }
      T update(T destination, Uint32 dT){
         T v;
         if(destination>value){
            v = accel;
            //cout<<"accel"<<endl;
         }else if (destination<value){
            v = decel;
            //cout<<"deccel"<<endl;
         }else{
            v = (T)0;
         }
         //vel *= (T)0.999f;//decay
         vel += v*(1.0f/dT);
         //cout<<"vel "<<vel;
         pos += vel;
         //cout<<"pos "<<pos;
         pos = clamp<T>(pos,min,max);
         value = pos*(max-min)+min;
         //cout<<"value "<<value<<endl;
         return value;        
      }
   private:
   T value, accel, decel, vel, min, max;
   float pos;

};

#endif
