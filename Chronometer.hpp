//Chrono class from OpenGA

#include <chrono>
#include <ctime>

class Chronometer
{
protected:
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> Timetype;
	Timetype time_start, time_stop;
	bool initialized;
public:

	Chronometer() : 
			initialized(false)
	{
	}

	void tic()
	{
		initialized=true;
		time_start = std::chrono::high_resolution_clock::now();
	}

	double toc()
	{
		if(!initialized)
			throw runtime_error("Chronometer is not initialized!");
	    time_stop = std::chrono::high_resolution_clock::now();
		return (double)std::chrono::duration<double>(time_stop-time_start).count();
	}

};