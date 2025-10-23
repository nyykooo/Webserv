#include "headers.hpp"

class WebServer;

class LocationBlock : public Block 
{
	private:
		bool													_exactMatchModifier;
		std::string												_location;
		static int												_locationCurlyBracketsCount;
		std::string												_uploadDirectory;
		long													_requestSize;
	public:

		// SETTERS

		void			setExactMatchModifier(bool value);
		void			setLocation(const std::string& location);
		static void		incrementLocationCurlyBracketsCount(void);
		static void		decrementLocationCurlyBracketsCount(void);
		void			setCgiMap(const std::string& extension, const std::string& path);
		void			setRequestSize(long reqSize);

		// GETTERS

		bool										getExactMatchModifier(void) const;
		const std::string&							getLocation(void) const;
		static int									getLocationCurlyBracketsCount(void);
		const std::map<std::string, std::string>&	getCgiMap(void) const;
		long										getRequestSize(void) const;

		// ORTHODOX CANONICAL FORM

		LocationBlock();
		LocationBlock(const Configuration&);
		~LocationBlock();
		LocationBlock(const LocationBlock& other);
		LocationBlock& operator=(const LocationBlock& other);
		
};

void	parseLocationBlock(std::ifstream& file, std::string& line,  LocationBlock& location);