#include <iostream>

#include "get.h"
#include "post.h"

int main()
{
	std::cout << "------------------ GET ------------------" << std::endl;
	get();
	std::cout << "------------------ POST ------------------" << std::endl;
	post();

	return EXIT_SUCCESS;
}