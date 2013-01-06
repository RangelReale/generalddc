#ifndef H__GENERALDDC_DEVICEIMPL_APPLE__H
#define H__GENERALDDC_DEVICEIMPL_APPLE__H

#include <GeneralDDC/Types.h>
#include <GeneralDDC/List.h>

#include <IOKit/IOKitLib.h>
#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/i2c/IOI2CInterface.h>

#include <string>

namespace GeneralDDC {

class DeviceImpl
{
public:
	DeviceImpl(CGDirectDisplayID displayid);
	virtual ~DeviceImpl();

	const std::string &getName();
	const std::string &getEDID();
	const std::string &getRawCapabilities();

	void readValue(code_t code, value_t *value, value_t *maximum);
	void writeValue(code_t code, value_t value);

	static void probe(List &list);
private:
    CGDirectDisplayID _displayid;
    IOI2CConnectRef _i2cconnection;
	std::string _name;
	std::string _edid;
	bool _capsloaded;
	std::string _caps;
    
	/*
	DeviceImpl();
	DeviceImpl(const DeviceImpl&);
	DeviceImpl& operator = (const DeviceImpl&);
	*/
};

};


#endif // H__GENERALDDC_DEVICEIMPL_APPLE__H