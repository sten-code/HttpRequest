#include <iostream>

#define GET_IMPL
#include "get.hpp"

#define POST_IMPL
#include "post.hpp"

int main()
{
	std::cout << "------------------ GET ------------------" << std::endl;
	get();
	std::cout << "------------------ POST ------------------" << std::endl;
	post();

	return EXIT_SUCCESS;
}
