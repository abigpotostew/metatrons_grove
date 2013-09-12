#include <climits>
#include <AL/alut.h>
#include <iostream>
#include <list>
#include <cstring>
#include <vector>
//#include <chrono>
#include <iomanip>

#include "audioin.hpp"

using namespace std;

#define FREQ 22050 
//#define CAPTURE_S 0.1 //how muhc buffer to keep in memory
//#define CAP_SIZE FREQ*CAPTURE_S
//#define CAPTURE_MS CAPTURE_S*1000

void handle_alc_error(ALenum error,const char* msg){
      if(error != ALC_NO_ERROR){
         cout<<msg<<endl;
               //exit(1);
                  }
}

/*******************
** PUBLIC
*******************/
AudioIn::AudioIn():samplesIn(0),avg_volume(0.0){
}

AudioIn::~AudioIn(){
	//cout<<"Cleaning up audio."<<endl;
	stop();
}

bool AudioIn::start(int capture_rate_milliseconds){
   this->capture_ms = capture_rate_milliseconds;
   this->capture_s = (float)this->capture_ms/1000.0;
   this->capture_size = FREQ*this->capture_s;
  // cout<<"Based on user capture rate of "<<capture_ms<<"ms, setting capture buffer to "<<capture_size<<endl;
   //capture_size=2048;
	alDistanceModel(AL_NONE);
    const ALCchar* devices = alcGetString(NULL,
          ALC_CAPTURE_DEVICE_SPECIFIER); 
	//TODO: Use default audio device. This only works for my setup such that
	//the usb is 2nd device in list.
    //cout<<"device 1: "<<devices<<endl;
    devices += strlen(devices)+1;
   //cout<<"device 2: "<<devices<<endl;
    inputDevice = alcCaptureOpenDevice(devices,FREQ,AL_FORMAT_MONO16,
          capture_size);//0.1s  buffer set here
    handle_alc_error(alcGetError(inputDevice),"ERROR IN OPENING DEVICE");
    alcCaptureStart(inputDevice); // Begin capturing
    handle_alc_error(alcGetError(inputDevice),"ERROR STARTING cAPTURING");
	buffer = vector<buffer_t>(FREQ);
	has_new_data = false;
   stopped = false;

	return true;
}

bool AudioIn::stop(){
	if(!stopped){
		stopped=true;
		//capture_thread.join();
    	alcCaptureStop(inputDevice);
    	alcCaptureCloseDevice(inputDevice);
		alcMakeContextCurrent(NULL);
	}
	return true;
}

bool AudioIn::update(time_T delta_time){
   //timer += delta_time;
   //if( timer >= capture_ms ){
      //cout<<"About to capture audio"<<endl;
      bool cap_success = this->capture();
      //cout<<"sccsfuly caught audio"<<endl;
      if( cap_success ) {
         //this->print_buffer();
         //timer = 0;
         return true;
      }
   //}
   return false;
}
template <class myType>
float normalize(myType val, myType min, myType max){
   //cout<<"normalizing "<<val<<" with range ["<<min<<" , "<<max<<"]"<<endl;
   float val_norm = val - min;
   float range = max - min;
   if ( range == 0.0 ) range = 2*max;
   return val_norm/range;
}


float AudioIn::get_average_level(){
   if( this->has_new_data ){
      this->has_new_data = false;
      int sum=0;
      auto b = this->get_buffer();
      buffer_t s;
      for(int i = 0; i < this->samplesIn; ++i){
         s=b[i];
         sum += (s<0)?(s+1)*-1:s;//absolute value of sample
      }
      buffer_t avg = sum/this->samplesIn;
      //cout<<"Found avg audio level: "<<avg<<endl;
      this->avg_volume = normalize(avg,(buffer_t)0,(buffer_t)32767);
   }
   return this->avg_volume;
}

vector<buffer_t> AudioIn::get_buffer(){
	if(stopped) {
		return vector<buffer_t>();
	}
	return buffer; //returns a copy of the buffer
}

void AudioIn::print_buffer(){
   auto b = this->get_buffer();
   buffer_t spectrum = samplesIn/10;
   //cout<<setw(7)<<b.size();
   for(buffer_t i=0;i<this->samplesIn;i+=spectrum){
      cout<<setw(6)<<b[i];
   }
   cout<<"\n"<<endl;
}

/*******************
** PRIVATE
*******************/
//Capture sound and put it in buffer, returns false if it's not yet time!
bool AudioIn::capture(){
    // Poll for captured audio
   //cout<<"polling for audio..."<<endl;
    alcGetIntegerv(inputDevice,ALC_CAPTURE_SAMPLES,(ALCsizei)sizeof(ALint),&samplesIn);

		//cout<<"samples in:"<<samplesIn<<endl;
    if(samplesIn>capture_size) {
       samplesIn=capture_size;
		//cout<<"samples in:"<<samplesIn<<endl;
		//cout<<"Audio Thread:"<<audioin_context->buffer[0]<<endl;
        // Grab the sound
      alcCaptureSamples(inputDevice,(ALCvoid*)buffer.data(),capture_size);
      this->has_new_data = true;
		return true;
	}else{
		return false;
	}
}
