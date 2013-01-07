#ifndef H__GENERALDDC_DEVICE__H
#define H__GENERALDDC_DEVICE__H

#include <GeneralDDC/Types.h>

#include <string>

namespace GeneralDDC {

class DeviceImpl;

class Device
{
public:
	enum Codes
	{
		POWER			= 0xE1,
		STANDBY			= 0xD6,
		INPUT_SOURCE	= 0x60,
		OSD				= 0xCA,
		VOLUME			= 0x62,
	};

	Device(DeviceImpl *impl);
	virtual ~Device();

	const std::string &getName();
	const std::string &getEDID();
	const std::string &getRawCapabilities();

	value_t readValue(code_t code);
	bool readValue(code_t code, value_t *value, value_t *maximum);
	bool writeValue(code_t code, value_t value);
private:
	DeviceImpl *_pImpl;
};

};


#endif // H__GENERALDDC_DDCDEVICE__H
