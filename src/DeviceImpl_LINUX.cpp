#include "GeneralDDC/DeviceImpl_LINUX.h"
#include "GeneralDDC/Exceptions.h"
#include "GeneralDDC/Device.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <set>
#include <dirent.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/i2c-dev.h>

/* ddc/ci defines */
#define DEFAULT_DDCCI_ADDR	0x37	/* ddc/ci logic sits at 0x37 */
#define DEFAULT_EDID_ADDR	0x50	/* edid sits at 0x50 */

#define DDCCI_COMMAND_READ	0x01	/* read ctrl value */
#define DDCCI_REPLY_READ	0x02	/* read ctrl value reply */
#define DDCCI_COMMAND_WRITE	0x03	/* write ctrl value */

#define DDCCI_COMMAND_SAVE	0x0c	/* save current settings */

#define DDCCI_REPLY_CAPS	0xe3	/* get monitor caps reply */
#define DDCCI_COMMAND_CAPS	0xf3	/* get monitor caps */
#define DDCCI_COMMAND_PRESENCE	0xf7	/* ACCESS.bus presence check */

/* control numbers */
#define DDCCI_CTRL_BRIGHTNESS	0x10

/* samsung specific, magictune starts with writing 1 to this register */
#define DDCCI_CTRL		0xf5
#define DDCCI_CTRL_ENABLE	0x0001
#define DDCCI_CTRL_DISABLE	0x0000

/* ddc/ci iface tunables */
#define MAX_BYTES		127	/* max message length */
#define DELAY   		45000	/* uS to wait after write */

#define CONTROL_WRITE_DELAY   80000	/* uS to wait after writing to a control (default) */

/* magic numbers */
#define MAGIC_1	0x51	/* first byte to send, host address */
#define MAGIC_2	0x80	/* second byte to send, ored with length */
#define MAGIC_XOR 0x50	/* initial xor for received frame */

namespace GeneralDDC {


/*
 * DeviceImpl
 */
DeviceImpl::DeviceImpl(const std::string &filename) : _filename(filename),
	_capsloaded(false)
{
	_fd = open(filename.c_str(), O_RDWR);
	if (!_fd)
		throwPError();
	
	try
	{
		readEDID();
	}
	catch (std::exception &e)
	{
		close(_fd);
		throw;
	}
}

DeviceImpl::~DeviceImpl()
{
	if (_fd)
		close(_fd);
}

const std::string &DeviceImpl::getName()
{
	return _edid;
}

const std::string &DeviceImpl::getEDID()
{
	return _edid;
}

const std::string &DeviceImpl::getRawCapabilities()
{
	if (!_capsloaded)
	{
		readCaps();
		_capsloaded = true;
	}

	return _caps;
}

void DeviceImpl::probe(List &list)
{
	char* filename = NULL;
	DIR *dirp;
	struct dirent *direntp;
	
	dirp = opendir("/dev/");
	
	while ((direntp = readdir(dirp)) != NULL)
	{
		if (!strncmp(direntp->d_name, "i2c-", 4))
		{
			std::string filename("/dev/");
			filename.append(direntp->d_name);

			try
			{
				DeviceImpl *impl = new DeviceImpl(filename);
				list.add(new Device(impl));
			}
			catch (std::exception &e)
			{
				// ignore
			}
		}
	}
	
	closedir(dirp);
}

void DeviceImpl::readValue(code_t code, value_t *value, value_t *maximum)
{
	unsigned short v, m;
	
	if (!readctrl(code, &v, &m))
		throw Exception("Could not read value");
	
	if (value) *value = v;
	if (maximum) *maximum = m;
}

void DeviceImpl::writeValue(code_t code, value_t value)
{
	if (!writectrl((unsigned char)code, (unsigned short)value, 0))
		throw Exception("Could not write value");
}

void DeviceImpl::readEDID()
{
	unsigned char buf[128];
	buf[0] = 0;	/* eeprom offset */
	
	if (i2c_write(DEFAULT_EDID_ADDR, buf, 1) > 0 &&
	    i2c_read(DEFAULT_EDID_ADDR, buf, sizeof(buf)) > 0) 
	{		
		if (buf[0] != 0 || buf[1] != 0xff || buf[2] != 0xff || buf[3] != 0xff ||
		    buf[4] != 0xff || buf[5] != 0xff || buf[6] != 0xff || buf[7] != 0)
		{
			throw Exception("Corrupted EDID at 0x%02x.");
			//return -1;
		}
		
		char pnpid[8];
		
		snprintf(pnpid, 8, "%c%c%c%02X%02X", 
			((buf[8] >> 2) & 31) + 'A' - 1, 
			((buf[8] & 3) << 3) + (buf[9] >> 5) + 'A' - 1, 
			(buf[9] & 31) + 'A' - 1, buf[11], buf[10]);
		_edid = pnpid;
		
		_digital = (buf[0x14] & 0x80);
		
		//return 0;
	} 
	else {
		throw Exception("Reading EDID 0x%02x failed.");
		//return -1;
	}
}

void DeviceImpl::readCaps()
{
	char *_raw_caps = (char*)malloc(16);
	int bufferpos = 0;
	unsigned char buf[64];	/* 64 bytes chunk (was 35, but 173P+ send 43 bytes chunks) */
	int offset = 0;
	int len, i;
	int retries = 3;
	
	do {
		_raw_caps[bufferpos] = 0;
		if (retries == 0) {
			//return -1;
			return;
		}
		
		len = raw_caps(offset, buf, sizeof(buf));
		if (len < 0) {
			retries--;
			continue;
		}
		
		if (len < 3 || buf[0] != DDCCI_REPLY_CAPS || (buf[1] * 256 + buf[2]) != offset) 
		{
			//if (!mon->probing || verbosity) {
				//fprintf(stderr, _("Invalid sequence in caps.\n"));
			//}
			retries--;
			continue;
		}

		_raw_caps = (char*)realloc(_raw_caps, bufferpos + len - 2);
		for (i = 3; i < len; i++) {
			_raw_caps[bufferpos++] = buf[i];
		}
		
		offset += len - 3;
		
		retries = 3;
	} while (len != 3);

	_raw_caps[bufferpos] = 0;	
	
	_caps = _raw_caps;
	
	free(_raw_caps);
}

int DeviceImpl::writectrl(unsigned char ctrl, unsigned short value, int delay)
{
	unsigned char buf[4];

	buf[0] = DDCCI_COMMAND_WRITE;
	buf[1] = ctrl;
	buf[2] = (value >> 8);
	buf[3] = (value & 255);

	int ret = ddcci_write(buf, sizeof(buf));
	
	/* Do the delay */
	if (delay > 0) {
		usleep(1000*delay);
	}
	/* Default delay : 80ms (anyway we won't get below 45ms (due to DELAY)) */
	else if (delay < 0) {
		usleep(CONTROL_WRITE_DELAY);
	}
	
	return ret;
}

/* return values: < 0 - failure, 0 - contron not supported, > 0 - supported */
int DeviceImpl::readctrl(unsigned char ctrl, unsigned short *value, unsigned short *maximum)
{
	unsigned char buf[8];

	int len = ddcci_raw_readctrl(ctrl, buf, sizeof(buf));
	
	if (len == sizeof(buf) && buf[0] == DDCCI_REPLY_READ &&	buf[2] == ctrl) 
	{	
		if (value) {
			*value = buf[6] * 256 + buf[7];
		}
		
		if (maximum) {
			*maximum = buf[4] * 256 + buf[5];
		}
		
		return !buf[1];
		
	}
	
	return -1;
}

void DeviceImpl::throwPError()
{
    throw Exception(std::string(strerror(errno)));
}

int DeviceImpl::i2c_write(unsigned int addr, unsigned char *buf, unsigned char len)
{
	int i;
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg             i2cmsg;

	/* done, prepare message */	
	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = 0;
	i2cmsg.len   = len;
	i2cmsg.buf   = (char*)buf;

	if ((i = ioctl(_fd, I2C_RDWR, &msg_rdwr)) < 0 )
	{
		throwPError();
		//return -1;
	}

	return i;
}

int DeviceImpl::i2c_read(unsigned int addr, unsigned char *buf, unsigned char len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg             i2cmsg;
	int i;

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = I2C_M_RD;
	i2cmsg.len   = len;
	i2cmsg.buf   = (char*)buf;

	if ((i = ioctl(_fd, I2C_RDWR, &msg_rdwr)) < 0)
	{
		throwPError();
		//return -1;
	}

	return i;
}

int DeviceImpl::ddcci_write(unsigned char *buf, unsigned char len)
{
	int i = 0;
	unsigned char _buf[MAX_BYTES + 3];
	unsigned int axor = ((unsigned char)DEFAULT_DDCCI_ADDR << 1);	/* initial xor value */

	/* put first magic */
	axor ^= (_buf[i++] = MAGIC_1);
	
	/* second magic includes message size */
	axor ^= (_buf[i++] = MAGIC_2 | len);
	
	while (len--) /* bytes to send */
		axor ^= (_buf[i++] = *buf++);
		
	/* finally put checksum */
	_buf[i++] = axor;

	/* wait for previous command to complete */
	ddcci_delay(1);

	return i2c_write(DEFAULT_DDCCI_ADDR, _buf, i);
}

int DeviceImpl::ddcci_read(unsigned char *buf, unsigned char len)
{
	unsigned char _buf[MAX_BYTES];
	unsigned char axor = MAGIC_XOR;
	int i, _len;

	/* wait for previous command to complete */
	ddcci_delay(0);

	if (i2c_read(DEFAULT_DDCCI_ADDR, _buf, len + 3) <= 0) /* busy ??? */
	{
		return -1;
	}
	
	/* validate answer */
	if (_buf[0] != DEFAULT_DDCCI_ADDR * 2) { /* busy ??? */
		throw Exception("Invalid response on first byte");
		//return -1;
	}

	if ((_buf[1] & MAGIC_2) == 0) {
		/* Fujitsu Siemens P19-2 and NEC LCD 1970NX send wrong magic when reading caps. */
		//if (!mon->probing || verbosity) {
			//fprintf(stderr, _("Non-fatal error: Invalid response, magic is 0x%02x\n"), _buf[1]);
		//}
	}

	_len = _buf[1] & ~MAGIC_2;
	if (_len > len || _len > sizeof(_buf)) {
		throw Exception("Invalid response on length");
		//return -1;
	}

	/* get the xor value */
	for (i = 0; i < _len + 3; i++) {
		axor ^= _buf[i];
	}
	
	if (axor != 0) {
		throw Exception("Invalid response, corrupted data");
		//return -1;
	}

	/* copy payload data */
	memcpy(buf, _buf + 2, _len);
		
	return _len;	
}

void DeviceImpl::ddcci_delay(int iswrite)
{
	struct timeval now;

	if (gettimeofday(&now, NULL)) {
		usleep(DELAY);
	} else {
		if (_last.tv_sec >= (now.tv_sec - 1)) {
			unsigned long usec = (now.tv_sec - _last.tv_sec) * 10000000 +
				now.tv_usec - _last.tv_usec;

			if (usec < DELAY) {
				usleep(DELAY - usec);
				if ((now.tv_usec += (DELAY - usec)) > 1000000) {
					now.tv_usec -= 1000000;
					now.tv_sec++;
				}
			}
		}
		
		if (iswrite) {
			_last = now;
		}
	}
}

int DeviceImpl::ddcci_raw_readctrl(unsigned char ctrl, unsigned char *buf, unsigned char len)
{
	unsigned char _buf[2];

	_buf[0] = DDCCI_COMMAND_READ;
	_buf[1] = ctrl;

	if (ddcci_write(_buf, sizeof(_buf)) < 0)
	{
		return -1;
	}

	return ddcci_read(buf, len);
}

int DeviceImpl::raw_caps(unsigned int offset, unsigned char *buf, unsigned char len)
{
	unsigned char _buf[3];

	_buf[0] = DDCCI_COMMAND_CAPS;
	_buf[1] = offset >> 8;
	_buf[2] = offset & 255;
	
	if (ddcci_write(_buf, sizeof(_buf)) < 0) 
	{
		return -1;
	}
	
	return ddcci_read(buf, len);
}

};
