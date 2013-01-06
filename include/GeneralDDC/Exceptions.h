#ifndef H__GENERALDDC_EXCEPTIONS__H
#define H__GENERALDDC_EXCEPTIONS__H

#include <stdexcept>
#include <string>

namespace GeneralDDC {

class Exception : public std::exception
{
public:
	Exception() : std::exception() {}
	Exception(const std::string &message) : std::exception(), _message(message) {}
	~Exception() throw() {}
	
	const char* what() const throw() { return _message.c_str(); }
private:
	std::string _message;
};

};


#endif // H__GENERALDDC_DDCEXCEPTIONS__H
