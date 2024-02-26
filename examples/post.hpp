#pragma once


void post();

#ifdef POST_IMPL
#include <iostream>
#include <http.hpp>

void post()
{
	http::Request req("https://httpbin.org/post");
	http::Response resp = req.post(
		{
			{"User-Agent", "Test User-Agent"}
		}, 
		"{\"test\": 69}"
	);
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
#endif
