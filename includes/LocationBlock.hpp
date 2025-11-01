#include "headers.hpp"

class WebServer;

class LocationBlock : public Block 
{
	private:
		bool													_exactMatchModifier;
		static int												_locationCurlyBracketsCount;
	public:

		// SETTERS

		void			setExactMatchModifier(bool value);
		static void		incrementLocationCurlyBracketsCount(void);
		static void		decrementLocationCurlyBracketsCount(void);

		// GETTERS

		bool										getExactMatchModifier(void) const;
		static int									getLocationCurlyBracketsCount(void);

		// ORTHODOX CANONICAL FORM

		LocationBlock();
		LocationBlock(const Configuration&);
		~LocationBlock();
		LocationBlock(const LocationBlock& other);
		LocationBlock& operator=(const LocationBlock& other);
		
};

void	parseLocationBlock(std::ifstream& file, std::string& line,  LocationBlock& location);