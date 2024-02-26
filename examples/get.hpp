#pragma once

void get();

#ifdef GET_IMPL
#include <iostream>
#include <http.hpp>

void get()
{
	http::Request req("https://httpbin.org/get");
	http::Response resp = req.get(
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
