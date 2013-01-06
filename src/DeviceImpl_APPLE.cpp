#include "GeneralDDC/DeviceImpl_APPLE.h"
#include "GeneralDDC/Exceptions.h"
#include "GeneralDDC/Device.h"

#include <iostream>
#include <sstream>

namespace GeneralDDC {

/*
 * DeviceImpl
 */
DeviceImpl::DeviceImpl(CGDirectDisplayID displayid) :
	_displayid(displayid), _i2cconnection(NULL), _capsloaded(false)
{
    std::stringstream s;
    s << _displayid;
    
    _name = s.str();
    _edid = s.str();
    
    
    kern_return_t kr;
    io_service_t  framebuffer, interface;
    IOOptionBits bus;
	IOItemCount busCount;
    io_string_t path;
    
    framebuffer = CGDisplayIOServicePort(_displayid);
    kr = IORegistryEntryGetPath(framebuffer, kIOServicePlane, path);
    assert( KERN_SUCCESS == kr );
    
    kr = IOFBGetI2CInterfaceCount( framebuffer, &busCount );
    assert( kIOReturnSuccess == kr );
    
    for( bus = 0; bus < busCount; bus++ )
    {
        IOI2CConnectRef  connect;
        
        kr = IOFBCopyI2CInterfaceForBus(framebuffer, bus, &interface);
        if( kIOReturnSuccess != kr)
            continue;
        
        kr = IOI2CInterfaceOpen( interface, kNilOptions, &connect );
        
        IOObjectRelease(interface);
        assert( kIOReturnSuccess == kr );
        if( kIOReturnSuccess != kr)
            continue;
        
        _i2cconnection = connect;
        break;
    }
    
    if (!_i2cconnection)
        throw Exception("Could not get IO connection");
    
}

DeviceImpl::~DeviceImpl()
{
    if (_i2cconnection)
        IOI2CInterfaceClose( _i2cconnection, kNilOptions );
}

const std::string &DeviceImpl::getName()
{
	return _name;
}

const std::string &DeviceImpl::getEDID()
{
	return _edid;
}

const std::string &DeviceImpl::getRawCapabilities()
{
	return _caps;
}

void DeviceImpl::readValue(code_t code, value_t *value, value_t *maximum)
{
    if (value) *value = 0;
    if (maximum) *maximum = 0;
}

void DeviceImpl::writeValue(code_t code, value_t value)
{
}

void DeviceImpl::probe(List &list)
{
    CGDisplayCount num_displays;
    CGGetActiveDisplayList(0, NULL, &num_displays);
    
    std::cout << num_displays << std::endl;
    
    CGDirectDisplayID* display_ids;
    display_ids = (CGDirectDisplayID*)malloc(sizeof(CGDirectDisplayID)*num_displays);
	CGGetActiveDisplayList(num_displays, display_ids, &num_displays);
    
    for(int i = 0; i < num_displays; i++)
	{
        try {
            std::cout << display_ids[i] << std::endl;
            
            list.add(new Device(new DeviceImpl(display_ids[i])));
        } catch (std::exception &e) {
            // ignore
        }
    }
    
    free(display_ids);
}


};
