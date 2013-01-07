#include <GeneralDDC/List.h>
#include <GeneralDDC/Exceptions.h>

#include <iostream>

int main(int argc, char* argv[])
{
	GeneralDDC::List list;
	list.probe();

	for (int i=0; i<list.count(); i++)
	{
		std::cout << list.at(i)->getName() << std::endl;
		//std::cout << list.at(i)->getRawCapabilities() << std::endl;
		try{
		std::cout << "POWER=" << list.at(i)->readValue(GeneralDDC::Device::POWER) << std::endl;
		} catch (GeneralDDC::Exception &e) {
			std::cout << "POWER ERROR=" << e.what() << std::endl;
		}
		try{
		std::cout << "INPUT SOURCE=" << list.at(i)->readValue(GeneralDDC::Device::INPUT_SOURCE) << std::endl;
		} catch (GeneralDDC::Exception &e) {
			std::cout << "INPUT SOURCE ERROR=" << e.what() << std::endl;
		}
	}

	return 0;
}