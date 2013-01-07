#ifndef H__GENERALDDC_DEVICEIMPL_LINUX__H
#define H__GENERALDDC_DEVICEIMPL_LINUX__H

#include <GeneralDDC/Types.h>
#include <GeneralDDC/List.h>

#include <string>

#include <time.h>
#include <sys/time.h>

namespace GeneralDDC {

class PhysicalMonitor;

class DeviceImpl
{
public:
	DeviceImpl(const std::string &filename);
	virtual ~DeviceImpl();

	const std::string &getName();
	const std::string &getEDID();
	const std::string &getRawCapabilities();

	bool readValue(code_t code, value_t *value, value_t *maximum);
	bool writeValue(code_t code, value_t value);

	static void probe(List &list);
private:
	void throwPError();
  
	void readEDID();
	void readCaps();
	
	int writectrl(unsigned char ctrl, unsigned short value, int delay);
	/* return values: < 0 - failure, 0 - contron not supported, > 0 - supported */
	int readctrl(unsigned char ctrl, unsigned short *value, unsigned short *maximum);
	
	int i2c_write(unsigned int addr, unsigned char *buf, unsigned char len);
	int i2c_read(unsigned int addr, unsigned char *buf, unsigned char len);
	int ddcci_write(unsigned char *buf, unsigned char len);
	int ddcci_read(unsigned char *buf, unsigned char len);
	void ddcci_delay(int iswrite);
	int ddcci_raw_readctrl(unsigned char ctrl, unsigned char *buf, unsigned char len);
	
	int raw_caps(unsigned int offset, unsigned char *buf, unsigned char len);
  
	std::string _filename;
	int _fd;
	unsigned char _digital;
	struct timeval _last;
	
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


#endif // H__GENERALDDC_DEVICEIMPL_LINUX__H