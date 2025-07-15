#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>


int main(void)
{
	std::time_t timestamp = std::time(&timestamp);
	std::tm *timeinfo = std::localtime(&timestamp);

    std::ostringstream oss;
	oss << (1900 + timeinfo->tm_year) << "-" << std::setfill('0') << std::setw(2) << (1 + timeinfo->tm_mon) << "-" << std::setfill('0') << std::setw(2) << timeinfo->tm_mday;

	std::cout << oss.str() << std::endl;
}
