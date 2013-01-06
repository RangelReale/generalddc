#include "GeneralDDC/DeviceImpl_WIN32.h"
#include "GeneralDDC/Exceptions.h"
#include "GeneralDDC/Device.h"

#include <PhysicalMonitorEnumerationAPI.h>
#include <lowlevelmonitorconfigurationapi.h>

namespace GeneralDDC {

std::string win32_to_utf8(const wchar_t* buffer)
{
        int nChars = ::WideCharToMultiByte(
                CP_UTF8,
                0,
                buffer,
                -1,
                NULL,
                0,
                NULL,
                NULL);
        if (nChars == 0) return "";

        std::string newbuffer;
        newbuffer.resize(nChars) ;
        ::WideCharToMultiByte(
                CP_UTF8,
                0,
                buffer,
                -1,
                const_cast< char* >(newbuffer.c_str()),
                nChars,
                NULL,
                NULL); 

        return newbuffer;
}


/*
 * PhysicalMonitor
 */

class PhysicalMonitor
{
public:
	PhysicalMonitor(const std::string &monitorname, DWORD count, LPPHYSICAL_MONITOR monitor);
	virtual ~PhysicalMonitor();

	void duplicate() { ++_counter; }
	void release() { if (--_counter == 0) delete this; }

	const std::string &getMonitorName() { return _monitorname; }
	int getCount() { return _count; }
	LPPHYSICAL_MONITOR getHandle() { return _monitor; }

	LPPHYSICAL_MONITOR getHandleAt(int index);
private:
	int _counter;
	std::string _monitorname;
	DWORD _count;
	LPPHYSICAL_MONITOR _monitor;
};


PhysicalMonitor::PhysicalMonitor(const std::string &monitorname, DWORD count, LPPHYSICAL_MONITOR monitor) : 
	_counter(1), _monitorname(monitorname), _count(count), _monitor(monitor)
{

}

PhysicalMonitor::~PhysicalMonitor()
{
	// Close the monitor handles.
	DestroyPhysicalMonitors(_count, _monitor);
	// Free the array.
	free(_monitor);
}

LPPHYSICAL_MONITOR PhysicalMonitor::getHandleAt(int index)
{
	return _monitor + index;
}


/*
 * DeviceImpl
 */
DeviceImpl::DeviceImpl(PhysicalMonitor *pmonitor, int index) : 
	_capsloaded(false), _pmonitor(pmonitor), _pmonitorindex(index)
{
	_pmonitor->duplicate();

	_name = win32_to_utf8(_pmonitor->getHandleAt(index)->szPhysicalMonitorDescription);
	_name.append(" - ");
	_name.append(pmonitor->getMonitorName());
}

DeviceImpl::~DeviceImpl()
{
	_pmonitor->release();
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
	if (!_capsloaded)
	{
		DWORD cchStringLength = 0;
		BOOL bSuccess = 0;
		LPSTR szCapabilitiesString = NULL;

		// Get the length of the string.
		bSuccess = GetCapabilitiesStringLength(
			_pmonitor->getHandleAt(_pmonitorindex)->hPhysicalMonitor, // Handle to the monitor.
		   &cchStringLength
		   );

		if (bSuccess)
		{
			// Allocate the string buffer.
			LPSTR szCapabilitiesString = (LPSTR)malloc(cchStringLength);
			if (szCapabilitiesString != NULL)
			{
				// Get the capabilities string.
				bSuccess = CapabilitiesRequestAndCapabilitiesReply(
					_pmonitor->getHandleAt(_pmonitorindex)->hPhysicalMonitor,
					szCapabilitiesString,
					cchStringLength
					);
				if (bSuccess)
					_caps = szCapabilitiesString;

				// Free the string buffer.
				free(szCapabilitiesString);
			}
		}

		_capsloaded = true;
	}

	return _caps;
}

void DeviceImpl::readValue(code_t code, value_t *value, value_t *maximum)
{
	if (!GetVCPFeatureAndVCPFeatureReply(_pmonitor->getHandleAt(_pmonitorindex)->hPhysicalMonitor, code, NULL, value, maximum))
		throw Exception("Could not read value");
}

void DeviceImpl::writeValue(code_t code, value_t value)
{
	if (!SetVCPFeature(_pmonitor->getHandleAt(_pmonitorindex)->hPhysicalMonitor, code, value))
		throw Exception("Could not write value");
}

BOOL CALLBACK MonitorEnumProc(
  _In_  HMONITOR hMonitor,
  _In_  HDC hdcMonitor,
  _In_  LPRECT lprcMonitor,
  _In_  LPARAM dwData
)
{
	List *list = (List *)dwData;

	MONITORINFOEX minfo;
	minfo.cbSize = sizeof(minfo);
	GetMonitorInfo(hMonitor, &minfo);

	std::string monitorname(minfo.szDevice);

	DWORD cPhysicalMonitors;
	LPPHYSICAL_MONITOR pPhysicalMonitors = NULL;

	// Get the number of physical monitors.
	BOOL bSuccess = GetNumberOfPhysicalMonitorsFromHMONITOR(
	  hMonitor, 
	  &cPhysicalMonitors
	   );

	if (bSuccess)
	{
		// Allocate the array of PHYSICAL_MONITOR structures.
		pPhysicalMonitors = (LPPHYSICAL_MONITOR)malloc(
			cPhysicalMonitors* sizeof(PHYSICAL_MONITOR));

		if (pPhysicalMonitors != NULL)
		{
			// Get the array.
			bSuccess = GetPhysicalMonitorsFromHMONITOR(
				hMonitor, cPhysicalMonitors, pPhysicalMonitors);
			if (bSuccess)
			{
				PhysicalMonitor *monitor = new PhysicalMonitor(monitorname, cPhysicalMonitors, pPhysicalMonitors);

			   // Use the monitor handles.
				LPPHYSICAL_MONITOR curPhysicalMonitors = pPhysicalMonitors;
				for (DWORD i=0; i<cPhysicalMonitors; i++)
				{
					DeviceImpl *impl = new DeviceImpl(monitor, i);
					list->add(new Device(impl));
					curPhysicalMonitors++;
				}

				monitor->release();
			}
		}
	}

	return TRUE;
}

void DeviceImpl::probe(List &list)
{
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&list);
}


};
