#ifndef H__GENERALDDC_EXCEPTIONS__H
#define H__GENERALDDC_EXCEPTIONS__H

#include <stdexcept>

namespace GeneralDDC {

class Exception : public std::exception
{
public:
	Exception() : std::exception() {}
	Exception(const char * const &message) : std::exception(message) {}
};

};


#endif // H__GENERALDDC_DDCEXCEPTIONS__H
