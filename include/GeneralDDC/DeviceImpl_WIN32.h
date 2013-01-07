#ifndef H__GENERALDDC_DEVICEIMPL_WIN32__H
#define H__GENERALDDC_DEVICEIMPL_WIN32__H

#include <GeneralDDC/Types.h>
#include <GeneralDDC/List.h>

#include <string>

#include <windows.h>

namespace GeneralDDC {

class PhysicalMonitor;

class DeviceImpl
{
public:
	DeviceImpl(PhysicalMonitor *pmonitor, int index);
	virtual ~DeviceImpl();

	const std::string &getName();
	const std::string &getEDID();
	const std::string &getRawCapabilities();

	bool readValue(code_t code, value_t *value, value_t *maximum);
	bool writeValue(code_t code, value_t value);

	static void probe(List &list);
private:
	std::string _name;
	std::string _edid;
	bool _capsloaded;
	std::string _caps;

	PhysicalMonitor *_pmonitor;
	int _pmonitorindex;

	/*
	DeviceImpl();
	DeviceImpl(const DeviceImpl&);
	DeviceImpl& operator = (const DeviceImpl&);
	*/
};

};


#endif // H__GENERALDDC_DEVICEIMPL_WIN32__H