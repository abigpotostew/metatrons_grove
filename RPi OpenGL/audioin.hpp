#ifndef AUDIOIN_HPP
#define AUDIOIN_hpp

#include <AL/alut.h>
#include <vector>
//#include <thread>

typedef short buffer_t;
typedef unsigned long long time_T;

using namespace std;

class AudioIn{
	friend void audioin_thread(AudioIn*);
public:
	AudioIn(); //spawn thread
	~AudioIn();
	bool start(int);
	bool stop();
	float get_average_level();
	vector<buffer_t> get_buffer();
   bool update(time_T delta_time);//returns
   void print_buffer();
private:
	vector<buffer_t> buffer; //Store our samples here
	ALCint samplesIn;
	ALCdevice* inputDevice; //The mix input device
	bool capture(); 
	bool has_new_data,stopped;
   float avg_volume;
   float capture_s; //seconds
   unsigned int capture_ms;//milliseconds
   ALCint capture_size;//size of buffer
	//thread capture_thread;
	AudioIn& operator=(const AudioIn&);
   time_T timer;
};

#endif //AUDIOIN_HPP
