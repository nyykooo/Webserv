
#include <iostream>
#include <ctime>
#include <string>

int main(void)
{
	std::time_t time = std::time(NULL);
	unsigned int starttime = time;
	std::cout << "start time: " << starttime << std::endl;
	std::cout << time << " seconds since the Epoch\n";
	unsigned int deltatime = 0;
	while (deltatime < 10)
	{
		time = std::time(NULL);
		deltatime = time - starttime;
	}
	std::cout << "elapsed time: " << deltatime << std::endl;
	std::cout << "totaltime: " << time << std::endl;
}
