#include "GeneralDDC/Device.h"
#include "GeneralDDC/Exceptions.h"

#include "GeneralDDC/DeviceImpl.h"

namespace GeneralDDC {

Device::Device(DeviceImpl *impl) : _pImpl(impl)
{

}

Device::~Device()
{
	if (_pImpl) delete _pImpl;
}

const std::string &Device::getName()
{
	return _pImpl->getName();
}

const std::string &Device::getEDID()
{
	return _pImpl->getEDID();
}

const std::string &Device::getRawCapabilities()
{
	return _pImpl->getRawCapabilities();
}

value_t Device::readValue(code_t code)
{
	value_t value;
	if (!_pImpl->readValue(code, &value, NULL))
		throw Exception("Error reading value");
	return value;
}

bool Device::readValue(code_t code, value_t *value, value_t *maximum)
{
	return _pImpl->readValue(code, value, maximum);
}

bool Device::writeValue(code_t code, value_t value)
{
	return _pImpl->writeValue(code, value);
}


};
