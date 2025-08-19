#include "../includes/headers.hpp"

HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse& other) {
	*this = other;
}

HttpResponse HttpResponse::operator=(const HttpResponse& other) {
	if (this != &other) {
		std::cout << std::endl;
	}
	return (*this);
}

HttpResponse::~HttpResponse() {}

// ### EXEC METHOD ###

void HttpResponse::openReg(std::string path, int methodType)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
	{
		std::cerr << "ERRO AO ABRIR FICHEIRO: " << strerror(errno) << std::endl;
		_resStatus = 404;
		return ;
	}
	_resStatus = 200;
	if (methodType == DELETE)
		return ;
	std::stringstream ss;

	ss << file.rdbuf();
	_resBody = ss.str();
	std::cout << "resBody: " << _resBody << std::endl;
	file.close();
	
}

void HttpResponse::setMimeTypes() {

    // Text
	_mimeTypes["html"]  = "text/html";
	_mimeTypes["htm"]   = "text/html";
	_mimeTypes["shtml"] = "text/html";
	_mimeTypes["css"]   = "text/css";
	_mimeTypes["xml"]   = "text/xml";
	_mimeTypes["txt"]   = "text/plain";
	_mimeTypes["mml"]   = "text/mathml";
	_mimeTypes["jad"]   = "text/vnd.sun.j2me.app-descriptor";
	_mimeTypes["wml"]   = "text/vnd.wap.wml";
	_mimeTypes["htc"]   = "text/x-component";

	// Images
	_mimeTypes["gif"]   = "image/gif";
	_mimeTypes["jpeg"]  = "image/jpeg";
	_mimeTypes["jpg"]   = "image/jpeg";
	_mimeTypes["avif"]  = "image/avif";
	_mimeTypes["png"]   = "image/png";
	_mimeTypes["svg"]   = "image/svg+xml";
	_mimeTypes["svgz"]  = "image/svg+xml";
	_mimeTypes["tif"]   = "image/tiff";
	_mimeTypes["tiff"]  = "image/tiff";
	_mimeTypes["wbmp"]  = "image/vnd.wap.wbmp";
	_mimeTypes["webp"]  = "image/webp";
	_mimeTypes["ico"]   = "image/x-icon";
	_mimeTypes["jng"]   = "image/x-jng";
	_mimeTypes["bmp"]   = "image/x-ms-bmp";

	// Fonts
	_mimeTypes["woff"]  = "font/woff";
	_mimeTypes["woff2"] = "font/woff2";

	// Applications
	_mimeTypes["js"]    = "application/javascript";
	_mimeTypes["atom"]  = "application/atom+xml";
	_mimeTypes["rss"]   = "application/rss+xml";
	_mimeTypes["jar"]   = "application/java-archive";
	_mimeTypes["war"]   = "application/java-archive";
	_mimeTypes["ear"]   = "application/java-archive";
	_mimeTypes["json"]  = "application/json";
	_mimeTypes["hqx"]   = "application/mac-binhex40";
	_mimeTypes["doc"]   = "application/msword";
	_mimeTypes["pdf"]   = "application/pdf";
	_mimeTypes["ps"]    = "application/postscript";
	_mimeTypes["eps"]   = "application/postscript";
	_mimeTypes["ai"]    = "application/postscript";
	_mimeTypes["rtf"]   = "application/rtf";
	_mimeTypes["m3u8"]  = "application/vnd.apple.mpegurl";
	_mimeTypes["kml"]   = "application/vnd.google-earth.kml+xml";
	_mimeTypes["kmz"]   = "application/vnd.google-earth.kmz";
	_mimeTypes["xls"]   = "application/vnd.ms-excel";
	_mimeTypes["eot"]   = "application/vnd.ms-fontobject";
	_mimeTypes["ppt"]   = "application/vnd.ms-powerpoint";
	_mimeTypes["odg"]   = "application/vnd.oasis.opendocument.graphics";
	_mimeTypes["odp"]   = "application/vnd.oasis.opendocument.presentation";
	_mimeTypes["ods"]   = "application/vnd.oasis.opendocument.spreadsheet";
	_mimeTypes["odt"]   = "application/vnd.oasis.opendocument.text";
	_mimeTypes["pptx"]  = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	_mimeTypes["xlsx"]  = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	_mimeTypes["docx"]  = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	_mimeTypes["wmlc"]  = "application/vnd.wap.wmlc";
	_mimeTypes["wasm"]  = "application/wasm";
	_mimeTypes["7z"]    = "application/x-7z-compressed";
	_mimeTypes["cco"]   = "application/x-cocoa";
	_mimeTypes["jardiff"] = "application/x-java-archive-diff";
	_mimeTypes["jnlp"]  = "application/x-java-jnlp-file";
	_mimeTypes["run"]   = "application/x-makeself";
	_mimeTypes["pl"]    = "application/x-perl";
	_mimeTypes["pm"]    = "application/x-perl";
	_mimeTypes["prc"]   = "application/x-pilot";
	_mimeTypes["pdb"]   = "application/x-pilot";
	_mimeTypes["rar"]   = "application/x-rar-compressed";
	_mimeTypes["rpm"]   = "application/x-redhat-package-manager";
	_mimeTypes["sea"]   = "application/x-sea";
	_mimeTypes["swf"]   = "application/x-shockwave-flash";
	_mimeTypes["sit"]   = "application/x-stuffit";
	_mimeTypes["tcl"]   = "application/x-tcl";
	_mimeTypes["tk"]    = "application/x-tcl";
	_mimeTypes["der"]   = "application/x-x509-ca-cert";
	_mimeTypes["pem"]   = "application/x-x509-ca-cert";
	_mimeTypes["crt"]   = "application/x-x509-ca-cert";
	_mimeTypes["xpi"]   = "application/x-xpinstall";
	_mimeTypes["xhtml"] = "application/xhtml+xml";
	_mimeTypes["xspf"]  = "application/xspf+xml";
	_mimeTypes["zip"]   = "application/zip";

	// Application/octet-stream group
	_mimeTypes["bin"]   = "application/octet-stream";
	_mimeTypes["exe"]   = "application/octet-stream";
	_mimeTypes["dll"]   = "application/octet-stream";
	_mimeTypes["deb"]   = "application/octet-stream";
	_mimeTypes["dmg"]   = "application/octet-stream";
	_mimeTypes["iso"]   = "application/octet-stream";
	_mimeTypes["img"]   = "application/octet-stream";
	_mimeTypes["msi"]   = "application/octet-stream";
	_mimeTypes["msp"]   = "application/octet-stream";
	_mimeTypes["msm"]   = "application/octet-stream";

	// Audio
	_mimeTypes["mid"]   = "audio/midi";
	_mimeTypes["midi"]  = "audio/midi";
	_mimeTypes["kar"]   = "audio/midi";
	_mimeTypes["mp3"]   = "audio/mpeg";
	_mimeTypes["ogg"]   = "audio/ogg";
	_mimeTypes["m4a"]   = "audio/x-m4a";
	_mimeTypes["ra"]    = "audio/x-realaudio";

	// Video
	_mimeTypes["3gpp"]  = "video/3gpp";
	_mimeTypes["3gp"]   = "video/3gpp";
	_mimeTypes["ts"]    = "video/mp2t";
	_mimeTypes["mp4"]   = "video/mp4";
	_mimeTypes["mpeg"]  = "video/mpeg";
	_mimeTypes["mpg"]   = "video/mpeg";
	_mimeTypes["ogv"]   = "video/ogg";
	_mimeTypes["mov"]   = "video/quicktime";
	_mimeTypes["webm"]  = "video/webm";
	_mimeTypes["flv"]   = "video/x-flv";
	_mimeTypes["m4v"]   = "video/x-m4v";
	_mimeTypes["mkv"]   = "video/x-matroska";
	_mimeTypes["mng"]   = "video/x-mng";
	_mimeTypes["asx"]   = "video/x-ms-asf";
	_mimeTypes["asf"]   = "video/x-ms-asf";
	_mimeTypes["wmv"]   = "video/x-ms-wmv";
	_mimeTypes["avi"]   = "video/x-msvideo";
}

const std::string	HttpResponse::getMimeType(const std::string& fileExtension) {
	size_t pos = fileExtension.find_last_of('.');
    
	if (pos == std::string::npos)
		return "application/octet-stream"; // default

	std::string ext = fileExtension.substr(pos + 1);

	for (size_t i = 0; i < ext.size(); ++i)
		ext[i] = std::tolower(ext[i]); // lowercase because it could be insensitive case

	std::map<std::string, std::string>::const_iterator it = _mimeTypes.find(ext);
	if (it != _mimeTypes.end())
		return it->second;

	return ("application/octet-stream");
}

static std::string createDirIndex(std::string path)
{
	std::string dirIndex = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Directory Index</title></head><body><h1>Directory Index for " + path + "</h1><ul>";

	DIR* dir = opendir(path.c_str());
	if (!dir) {
		std::cerr << "Failed to open directory: " << strerror(errno) << std::endl;
		return "<p>Error opening directory.</p>";
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') { // Skip hidden files
			dirIndex += "<li>" + std::string(entry->d_name) + "</li>";
		}
	}

	dirIndex += "</ul></body></html>";
	closedir(dir);
	return dirIndex;
}

void	HttpResponse::openDir(std::string path)
{
	std::string newPath = removeSlashes(path);
	for (std::vector<std::string>::const_iterator it = _block->getDefaultFiles().begin(); it != _block->getDefaultFiles().end(); ++it)
	{
		std::string newDefault = removeSlashes(*it);
		std::string filePath = newPath + "/" + newDefault;
		std::ifstream file(filePath.c_str());
		if (file.is_open())
		{
			std::stringstream ss;
			ss << file.rdbuf();
			_resBody = ss.str();
			file.close();
			_resStatus = 200;
			return ;
		}
	}

	switch (_block->getAutoIndex())
	{
		case false:
			_resStatus = 403; // mudar para 403
			return ;
		case true:
			_resBody = createDirIndex(path);
			_resStatus = 200;
			break;
	}
}

std::string HttpResponse::getFullPath () {
	std::string locPath = _req->getPath();
	if (_block->getRoot().empty())
		return (locPath);
	if (_loc != NULL){
		size_t locIndex = _req->getPath().find(_loc->getLocation());
		
		if (locIndex == 0)
		{
			locPath = _req->getPath().substr(_loc->getLocation().size());
			if (locPath[0] == '/')
				locPath.erase(0, 1);
		}
	}
	return (locPath);
}

void	HttpResponse::checkFile(int methodType) {
	struct stat st;
	if (stat(_fileName.c_str(), &st) == -1)
	{
		std::cerr << "stat error:" << strerror(errno) << std::endl;
		_resStatus = 404;
		return ;
	}
	if (S_ISREG(st.st_mode)) {
		openReg(_fileName, methodType);	
	}
	else if (S_ISDIR(st.st_mode))
	{
		std::cout << "Diretório encontrado: " << _fileName << std::endl;
		if (methodType == GET)
			openDir(_fileName);
		else if (methodType == DELETE)
			_resStatus = 409;
	}
	else if (S_ISLNK(st.st_mode))
		std::cout << "Link simbólico\n";
	else
		std::cout << "Outro tipo de arquivo\n";
}

void	HttpResponse::handleGET()
{
	if (!(_block->getNewLocation().empty())) {
		_resStatus = _block->getRedirectStatusCode();
		return ;
	}
	std::string	newRoot = removeSlashes(_conf->getRoot());
	std::string locPath = removeSlashes(this->getFullPath());
	if (!newRoot.empty())
		newRoot = "/" + newRoot;
	std::string _fileName = newRoot + "/" + locPath;
	std::cout << "GET file: " << _fileName << std::endl;
	checkFile(GET);
}

LocationBlock* HttpResponse::checkLocationBlock() {
	std::vector<LocationBlock>::iterator it; 
	size_t			longestMatch;
	LocationBlock	*tempLocation = NULL;

	it = _conf->locations.begin();
	longestMatch = 0;
	while (it != _conf->locations.end())
	{
		const std::string&	locationPath = it->getLocation();
		const std::string&	requestPath = _req->getPath();
		if (requestPath.compare(0, locationPath.size(), locationPath) == 0) {
			if (locationPath.size() > longestMatch) {
				longestMatch = locationPath.size();
				tempLocation = &(*it);
			}
		}
		it++;
	}
	return (tempLocation);
}

void HttpResponse::handleDELETE() {
	if (!(_block->getNewLocation().empty())) {
		_resStatus = _block->getRedirectStatusCode();
		return ;
	}
	std::string	newRoot = removeSlashes(_conf->getRoot());
	std::string locPath = removeSlashes(this->getFullPath());
	if (!newRoot.empty())
		newRoot = "/" + newRoot;
	std::string _fileName = newRoot + "/" + locPath;
	checkFile(DELETE);
	if (_resStatus != 200)
		return ;
	std::cout << _fileName << std::endl;
	int removed = std::remove(_fileName.c_str());
	if (removed == 0)
		_resStatus = 204;
	else
		_resStatus = 403;	
}

void	HttpResponse::execMethod()
{
	bool methodFound = false;
	std::string root;
	std::string	method = _req->getMethod();
	std::vector<std::string>::const_iterator it;

	for (it = _block->getMethods().begin(); it != _block->getMethods().end(); ++it) {
		if (*it != "GET" && *it != "POST" && *it != "DELETE") {
			_resStatus = 501;
			return ;
		}
		if (*it == method)
			methodFound = true;
	}
	if (methodFound == false)
	{
		_resStatus = 405; // Method Not Allowed
		return ;
	}
	root = _block->getRoot();

	if (method == "GET")
		handleGET();
	else if (method == "DELETE")
		handleDELETE();
}

static const std::string& http_error_400_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>400</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Bad Request</h1>"
"</body>"
"</html>";

static const std::string& http_error_403_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>403</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Forbidden</h1>"
"</body>"
"</html>";

static const std::string& http_error_404_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>404</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Page not found</h1>"
"</body>"
"</html>";

static const std::string& http_error_405_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>405</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Method Not Allowed</h1>"
"</body>"
"</html>";

static const std::string& http_error_408_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>408</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Request Timeout</h1>"
"</body>"
"</html>";

static const std::string& http_error_409_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>409</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Conflict</h1>"
"</body>"
"</html>";

static const std::string& http_error_411_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>411</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Length Required</h1>"
"</body>"
"</html>";

static const std::string& http_error_413_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>413</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Payload Too Large</h1>"
"</body>"
"</html>";

static const std::string& http_error_414_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>414</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>URI Too Long</h1>"
"</body>"
"</html>";


static const std::string& http_error_500_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>500</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Internal Server Error</h1>"
"</body>"
"</html>";

static const std::string& http_error_501_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>501</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Not implemented</h1>"
"</body>"
"</html>";

static const std::string& http_error_503_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>503</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Service Unavailable</h1>"
"</body>"
"</html>";

static const std::string& http_error_504_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>504</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Gateway Timeout</h1>"
"</body>"
"</html>";

static const std::string& http_error_505_page =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>505</title>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
"h1 { color: #333; }"
"</style>"
"</head>"
"<body>"
"<h1>Version Not Supported</h1>"
"</body>"
"</html>";



const std::string HttpResponse::httpFileContent(int errorPage) {
	
	char buffer[BUFFER_SIZE];
	std::string result;
	ssize_t bytesRead;
	while ((bytesRead = read(errorPage, buffer, BUFFER_SIZE)) > 0) {
		result.append(buffer, bytesRead);
	}
	if (bytesRead < 0) {
		_resStatus = 404;
        return http_error_404_page;
	}
	return (result);
}

int HttpResponse::openFile() {

	std::set<ErrorPageRule>::const_iterator it = _conf->getErrorPage().begin();

	while (it != _conf->getErrorPage().end()) {
		if ((*it).error == _resStatus)
			break ;
		it++;
	}
	if (it == _conf->getErrorPage().end())
		return (0);
		
	int	configFile = open((*it).errorPath.c_str(), O_RDONLY);
	if (configFile < 0)
		return (-1);
	return (configFile);
}

std::string	HttpResponse::header(const std::string& status) {

    std::ostringstream header;
    header << "HTTP/1.1 " << status << CRLF;
	header << "Server: MyServer/1.0" << CRLF;
	header << "Date: " << get_http_date() << CRLF;
	header << "Content-Type: " << getMimeType(_fileName) << CRLF;
    header << "Content-Length: " << _resBody.size() << CRLF;
    header << CRLF;

	return (header.str());
}

std::string setHeader(const std::string& str) {
	std::ostringstream response;
	
	response << "HTTP/1.1" << str << CRLF;
	response << "Date: " << get_http_date() << CRLF;
	response << "Server: MyServer/1.0" << CRLF;
	response << CRLF;

	return (response.str());
}

const std::string	HttpResponse::setRedirectHeader(const std::string& str) {
	std::ostringstream response;
	
	response << "HTTP/1.1" << str << CRLF;
	response << "Server: MyServer/1.0" << CRLF;
	response << "Date: " << get_http_date() << CRLF;
	response << "Content-Type: " << getMimeType(_fileName) << CRLF;
	response << "Location: " << _block->getNewLocation() << CRLF;
	response << CRLF;

	_resBody = "<html>";
	_resBody += "<head><title>";
	_resBody += str;
	_resBody += "</title></head>";
	_resBody += "<body>";
	_resBody += "<center><h1>";
	_resBody += str;
	_resBody += "</h1></center>";
	_resBody += "<hr><center>MyServer/1.0</center>";
	_resBody += "</body>";
	_resBody += "</html>";

	return (response.str() + _resBody);
}

const std::string HttpResponse::checkErrorResponse(const std::string& httpStatus, const std::string& page) {
	int	errorPage;

	errorPage = openFile();
	if (errorPage < 0) {
		_resBody = http_error_404_page;
		return (header(ERROR_404) + _resBody);
	}
	else if (errorPage == 0)
		_resBody = page;
	else
		_resBody = httpFileContent(errorPage);
	httpStatus.c_str();
	return (header(httpStatus) + _resBody);
}

const std::string HttpResponse::checkStatusCode() {
	std::string fileContent;
	
	std::cout << "status: " << _resStatus << std::endl;
	// esta função so valida caso o error_page esteja definido no .config file. tem de ser ajustada para caso não exista.
	switch (_resStatus) {
		case 200:
			if (_method == DELETE || _method == POST)
				return (setHeader("200 OK"));
			return (header("200 OK") + _resBody);
		case 202:
			return ("HTTP/1.1 202 Accepted");
		case 204:
			return (setHeader("204 No Content"));
		case 301:
			return (setRedirectHeader("301 Moved Permanently"));
		case 302:
			return (setRedirectHeader("302 Found"));
		case 303:
			return (setRedirectHeader("303 See Other"));
		case 304:
			return (setRedirectHeader("304 Not Modified"));
		case 307:
			return (setRedirectHeader("307 Temporary Redirect"));
		case 308:
			return (setRedirectHeader("308 Permanent Redirect"));
		case 400:
			return (checkErrorResponse("400 Bad Request", http_error_400_page));
		case 403:
			return (checkErrorResponse("403 Forbidden", http_error_403_page));
		case 404:
			return (checkErrorResponse("404 Not Found", http_error_404_page));
		case 405:
			return (checkErrorResponse("405 Method Not Allowed", http_error_405_page));
		case 408:
			return (checkErrorResponse("408 Request Timeout", http_error_408_page));
		case 409:
			return (checkErrorResponse("409 Conflict", http_error_409_page));
		case 411:
			return (checkErrorResponse("411 Length Required", http_error_411_page));
		case 413:
			return (checkErrorResponse("413 Payload Too Large", http_error_413_page));
		case 414:
			return (checkErrorResponse("414 URI Too Long", http_error_414_page));
		case 500:
			return (checkErrorResponse("500 Internal Server error", http_error_500_page));
		case 501:
			return (checkErrorResponse("501 Not Implemented", http_error_501_page));
		case 503:
			return (checkErrorResponse("503 Service Unavailable", http_error_503_page));
		case 504:
			return (checkErrorResponse("504 Gateway Timeout", http_error_504_page));
		case 505:
			return (checkErrorResponse("505 HTTP Version Not Supported", http_error_505_page));
	}
	std::cout << "Status code not handled: " << _resStatus << std::endl;
	return _resBody;
}

HttpResponse::HttpResponse(HttpRequest *request, Configuration *config):_method(-1) {
	_conf = config;
	_req = request;
	_loc = checkLocationBlock();
	if (_loc != NULL)
		_block = _loc;	
	else
		_block = _conf;
	std::string	pageContent;


	setMimeTypes();
	execMethod();
	pageContent = checkStatusCode();
	//std::cout << YELLOW << pageContent << RESET << std::endl;
	setResponse(pageContent);
}

// ### SETTERS ###

void HttpResponse::setResponse(const std::string& response) {
	_response = response;
}

const std::string&	HttpResponse::getResponse(void) const {
	return (_response);
}

const Configuration&	HttpResponse::getConfig(void) const {
	return (*_conf);
}
