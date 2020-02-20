#include "SystemManager.h"

#include <iostream>

int main()
{
	SystemManager s;

	try
	{
		if (!s.initialize())
			throw std::runtime_error("Failed to initialize system manager");
		if (!s.run())
			throw std::runtime_error("Failed to run system manager");
		if (!s.cleanup())
			throw std::runtime_error("Failed to cleanup system manager");
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
#ifdef _WIN32
		system("PAUSE");
#endif
		return EXIT_FAILURE;
	}

	std::cout << "Successful exit !" << std::endl;

#ifdef _WIN32
	system("PAUSE");
#endif
	return EXIT_SUCCESS;
}
