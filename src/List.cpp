#include "GeneralDDC/List.h"
#include "GeneralDDC/Exceptions.h"

#include "GeneralDDC/DeviceImpl.h"

#include <vector>

namespace GeneralDDC {


class ListImpl
{
public:
	ListImpl()
	{

	}

	~ListImpl()
	{
		for (std::vector<Device*>::iterator i=_list.begin(); i!=_list.end(); i++)
		{
			delete *i;
		}
	}

	void add(Device *device)
	{
		_list.push_back(device);
	}

	int count()
	{
		return _list.size();
	}

	Device *at(int index)
	{
		return _list.at(index);
	}
private:
	std::vector<Device*> _list;
};




List::List() : _pImpl(new ListImpl)
{

}

List::~List()
{
	delete _pImpl;
}

void List::add(Device *device)
{
	_pImpl->add(device);
}

int List::count()
{
	return _pImpl->count();
}

Device *List::at(int index)
{
	return _pImpl->at(index);
}

void List::probe()
{
	DeviceImpl::probe(*this);
}

};
