#ifndef H__GENERALDDC_LIST__H
#define H__GENERALDDC_LIST__H

#include <GeneralDDC/Device.h>

#include <string>

namespace GeneralDDC {

class ListImpl;

class List
{
public:
	List();
	virtual ~List();

	void add(Device *device);
	int count();
	Device *at(int index);

	void probe();
private:
	ListImpl *_pImpl;
};

};


#endif // H__GENERALDDC_LIST__H
