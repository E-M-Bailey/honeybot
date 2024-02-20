#include <iostream>

int main(int argc, char const *argv[])
{
	if (argc != 6)
	{
		std::cerr << "Expected 5 arguments.\n";
		return 1;
	}

	std::cout << "Hello, world!\n";
	for (int i = 0; i < argc; i++)
		std::cout << i << ' ' << argv[i] << '\n';

	return 0;
}
