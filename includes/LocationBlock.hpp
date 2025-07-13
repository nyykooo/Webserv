#include "headers.hpp"

class WebServer;

class LocationBlock {
	private:
		std::string					_root;
		std::vector<std::string>	_allowedMethods;
		bool						_autoIndex;
		bool						_exactMatchModifier;
		std::string					_location;
		int							_redirectStatusCode;
		std::string					_newLocation;
		std::vector<std::string>	_cgiExtension;
		std::vector<std::string>	_cgiPath;
		static int					_locationCurlyBracketsCount;
	public:

	// SETTERS

	void			setRoot(const std::string& root);
	void			setAllowedMethods(const std::string& method);
	void			removeAllowedMethods(void);
	void			setAutoIndex(const std::string& value);
	void			setRedirectStatusCode(const std::string& statusCode);
	void			setNewLocation(const std::string& newLocation);
	void			setExactMatchModifier(bool value);
	void			setLocation(const std::string& location);
	static void		incrementLocationCurlyBracketsCount(void);
	static void		decrementLocationCurlyBracketsCount(void);


	// GETTERS

	const std::string&				getRoot(void) const;
	const std::vector<std::string>&	getMethods(void) const;
	bool							getAutoIndex(void) const;
	bool							getExactMatchModifier(void) const;
	int								getStatusCode(void) const;
	const std::string&				getNewLocation(void) const;
	const std::string&				getLocation(void) const;
	static int						getLocationCurlyBracketsCount(void);

	// ORTHODOX CANONICAL FORM

	LocationBlock();
	~LocationBlock();
	LocationBlock(const LocationBlock& other);
	LocationBlock& operator=(const LocationBlock& other);
		
};

void	parseLocationBlock(std::ifstream& file, std::string& line,  LocationBlock& location);