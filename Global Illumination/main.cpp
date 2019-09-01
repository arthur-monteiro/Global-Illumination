#include "System.h"

#include <iostream>

int main()
{
	{
		System s;

		try
		{
			s.initialize();
			s.mainLoop();
			s.cleanup();
		}
		catch (const std::runtime_error& e)
		{
			std::cerr << e.what() << std::endl;
			system("PAUSE");
			return EXIT_FAILURE;
		}
	}

#ifdef _WIN32
	system("PAUSE");
#endif
	return EXIT_SUCCESS;
}