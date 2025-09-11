#include "headers.hpp"

class WebServer;
class  ErrorPageRule;

class LocationBlock : public Block 
{
	private:
		bool													_exactMatchModifier;
		std::string												_location;
		std::map<std::string, std::string>						_cgiPath;
		static int												_locationCurlyBracketsCount;
		std::set<ErrorPageRule>									_errorPage;
		std::string												_uploadDirectory;
	public:

	// SETTERS

	void			setExactMatchModifier(bool value);
	void			setLocation(const std::string& location);
	void			setErrorPage(int errorPage, const std::string& errorPagePath, int newStatus);
	static void		incrementLocationCurlyBracketsCount(void);
	static void		decrementLocationCurlyBracketsCount(void);
	void			setCgiPath(const std::string& extension, const std::string& path);
	void			setUploadDirectory(const std::string& str);

	// GETTERS

	bool										getExactMatchModifier(void) const;
	const std::string&							getLocation(void) const;
	const std::set<ErrorPageRule>&				getErrorPage(void) const;
	static int									getLocationCurlyBracketsCount(void);
	const std::map<std::string, std::string>&	getCgiPath(void) const;
	const std::string&							getUploadDirectory(void) const;

	// ORTHODOX CANONICAL FORM

	LocationBlock();
	LocationBlock(const Configuration&);
	~LocationBlock();
	LocationBlock(const LocationBlock& other);
	LocationBlock& operator=(const LocationBlock& other);
		
};

void	parseLocationBlock(std::ifstream& file, std::string& line,  LocationBlock& location);