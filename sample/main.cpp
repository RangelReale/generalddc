#include <GeneralDDC/List.h>

#include <iostream>

int main(int argc, char* argv[])
{
	GeneralDDC::List list;
	list.probe();

	for (int i=0; i<list.count(); i++)
	{
		std::cout << list.at(i)->getName() << std::endl;
		//std::cout << list.at(i)->getRawCapabilities() << std::endl;
		std::cout << "POWER=" << list.at(i)->readValue(GeneralDDC::Device::POWER) << std::endl;
		std::cout << "INPUT SOURCE=" << list.at(i)->readValue(GeneralDDC::Device::INPUT_SOURCE) << std::endl;
	}

	return 0;
}