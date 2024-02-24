#include <iostream>

#include <http.hpp>

int main()
{
	http::Request req("https://httpbin.org/get");
	http::Response resp = req.get();
	if (resp.success())
	{
		// Get the status line
		std::cout << resp.protocol() << " " << resp.status() << " " << resp.status_message() << std::endl;

		// Get the headers
		for (const auto& header : resp.headers())
			std::cout << header.first << ": " << header.second << std::endl;

		// Get the body as an std::string
		std::cout << resp.body() << std::endl;
	}
}