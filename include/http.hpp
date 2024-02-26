#pragma once

#if defined(_WIN32) || defined(WIN32)
#define PLATFORM_WINDOWS
#elif defined(__linux__)
#define PLATFORM_LINUX
#endif

#ifdef PLATFORM_WINDOWS
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

typedef int socklen_t;
#define RECVFROM_BUFFER char
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define AF_INET6 23

#define STARTWSA()																\
	WSADATA wsaData;                                                            \
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)								\
	{																			\
		throw std::runtime_error("Winsock initialization failed");				\
	}

#pragma comment (lib, "crypt32")
#pragma comment(lib, "legacy_stdio_definitions")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

#endif // PLATFORM_WINDOWS

#ifdef PLATFORM_LINUX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#define WSACleanup()
#define WSAGetLastError() ""
#define WSADATA int
#define STARTWSA()
#define MAKEWORD()
#define SOCKET int
#define SOCKADDR sockaddr
#define closesocket(x) close(x)
#define INVALID_SOCKET 0
#define SOCKET_ERROR -1
#define RECVFROM_BUFFER void

#endif // PLATFORM_LINUX

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <sstream>
#include <unordered_map>

namespace http
{
	enum Error
	{
		None = 0,
		InvalidURL,
		InvalidResponse,
		FailedAddressResolve,
		FailedSocketCreation,
		FailedConnection,
		FailedSSLConnection,
		FailedSendRequest,
		FailedReceiveResponse
	};

	[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Error& orig)
	{
		switch (orig)
		{
		case None: os << "None"; break;
		case InvalidURL: os << "Invalid URL"; break;
		case InvalidResponse: os << "Invalid Response"; break;
		case FailedAddressResolve: os << "Failed to parse url"; break;
		case FailedSocketCreation: os << "Failed to create socket"; break;
		case FailedConnection: os << "Failed to connect to host"; break;
		case FailedSSLConnection: os << "Failed to connect ssl"; break;
		case FailedSendRequest: os << "Failed to send request"; break;
		case FailedReceiveResponse: os << "Failed to receive response"; break;
		default:
			os << "Unknown error";
			return os;
		}
		os << " (" << (int)orig << ")";
		return os;
	}

	struct Response
	{
	public:
		Response(const std::string& response)
		{
			size_t bodyStart = response.find("\r\n\r\n");
			if (bodyStart == std::string::npos)
			{
				m_error = Error::InvalidResponse;
				return;
			}
			bodyStart += 4; // Skip "\r\n\r\n"

			std::istringstream iss(response.substr(0, bodyStart));

			// Parse status line
			{
				std::getline(iss, m_statusline);
				std::istringstream statuslineIss(m_statusline);

				statuslineIss >> m_protocol >> m_status;
				statuslineIss.ignore(1); // Skip the whitespace
				std::getline(statuslineIss, m_status_message);
			}

			// Parse headers
			{
				std::string headerLine;
				while (std::getline(iss, headerLine) && !headerLine.empty())
				{
					size_t delimiterPos = headerLine.find(": ");
					if (delimiterPos != std::string::npos)
					{
						std::string key = headerLine.substr(0, delimiterPos);
						std::string value = headerLine.substr(delimiterPos + 2, headerLine.size() - (delimiterPos + 3)); // Strip the last \r from it as std::getline doesn't
						m_headers[key] = value;
					}
				}
			}

			// The body length is defined with content-length
			if (m_headers.find("Content-Length") != m_headers.end())
			{
				std::string contentLength = m_headers["Content-Length"];
				size_t length = std::stoul(contentLength);
				m_body = response.substr(bodyStart, length);
			}
			// The body has been chunk encoded
			else if (m_headers.find("Transfer-Encoding") != m_headers.end() && m_headers["Transfer-Encoding"] == "chunked")
			{
				// Parse body
				std::string chunkedData = response.substr(bodyStart);
				while (!chunkedData.empty())
				{
					size_t chunkSizePos = chunkedData.find("\r\n");
					if (chunkSizePos == std::string::npos)
					{
						m_error = Error::InvalidResponse;
						break;
					}
					std::string chunkSizeStr = chunkedData.substr(0, chunkSizePos);
					size_t chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
					if (chunkSize == 0)
						break;
					size_t chunkDataStart = chunkSizePos + 2; // Skip "\r\n"
					size_t chunkDataEnd = chunkDataStart + chunkSize;
					if (chunkDataEnd > chunkedData.size())
					{
						m_error = Error::InvalidResponse;
						break;
					}
					std::string chunk = chunkedData.substr(chunkDataStart, chunkSize);
					m_body += parse_chunk(chunk, chunkSize);
					chunkedData = chunkedData.substr(chunkDataEnd + 2); // Skip chunk data and "\r\n"
				}
			}
		}

		Response(Error error)
			: m_error(error)
		{}

		// An error that occurs while sending, receiving and parsing the request, Error::None by default.
		inline Error error() const { return m_error; }

		// If an error didn't occur while sending, receiving and parsing the request and if the status code is 200, this will return true.
		inline bool success() const { return m_status == 200 && m_error == Error::None; }

		// The status line, including, protocol, status code and status mesage, for example: "HTTP/1.1 200 OK"
		inline std::string statusline() const { return m_statusline; }

		// The protocol, for example: "HTTP/1.1"
		inline std::string protocol() const { return m_protocol; }

		// The status code, for example: 200 or 404
		inline uint16_t status() const { return m_status; }

		// The status message, for example: "OK"
		inline std::string status_message() const { return m_status_message; }

		// The http headers of the response
		inline std::unordered_map<std::string, std::string>& headers() { return m_headers; }

		// The body of the response
		inline std::string body() const { return m_body; }
	private:
		std::string parse_chunk(const std::string& chunk, size_t size)
		{
			std::istringstream iss(chunk);
			std::string data;
			data.resize(size);
			iss.read(&data[0], size);
			return data;
		}
	private:
		Error m_error = Error::None;		// An error that occurs while sending, receiving and parsing the request, Error::None by default.

		std::string m_statusline = "";		// The status line, including, protocol, status code and status mesage, for example: "HTTP/1.1 200 OK"
		std::string m_protocol = "";		// The protocol, for example: "HTTP/1.1"
		uint16_t m_status = 0;				// The status code, for example: 200 or 404
		std::string m_status_message = "";	// The status message, for example: "OK"

		std::unordered_map<std::string, std::string> m_headers = {};	// The http headers of the response

		std::string m_body = "";			// The body of the response
	};

	class Request
	{
	public:
		Request(const std::string& url)
			: m_url(url)
		{
			// Extract the host and protocol
			size_t pos = url.find("://");
			if (pos != std::string::npos)
			{
				m_protocol = url.substr(0, pos);
				pos += 3; // Move past "://"
				std::string next = url.substr(pos, url.size() - pos);
				size_t end = next.find("/");
				if (end == std::string::npos)
				{
					m_host = next;
					m_path = "";
				}
				else
				{
					m_host = next.substr(0, end);
					m_path = next.substr(end, next.size() - end);
				}
			}
			else
			{
				//std::cerr << "Invalid URL format" << std::endl;
				return;
			}

			if (m_protocol == "https")
				m_port = "443";
			else if (m_protocol == "http")
				m_port = "80";

			STARTWSA();

			SSL_library_init();
			SSL_load_error_strings();
			OpenSSL_add_all_algorithms();

			m_result = nullptr;
			struct addrinfo hints;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			if (getaddrinfo(m_host.c_str(), m_port.c_str(), &hints, &m_result) != 0)
			{
				//std::cerr << "Failed to resolve address" << std::endl;
				WSACleanup();
				return;
			}
		}

		~Request()
		{
			free();
		}

		void free()
		{
			freeaddrinfo(m_result);
			WSACleanup();
		}

		Response send(std::string type, std::unordered_map<std::string, std::string> headers = {}, std::string body = "")
		{
			m_sock = socket(m_result->ai_family, m_result->ai_socktype, m_result->ai_protocol);
			if (m_sock == INVALID_SOCKET)
				return Response(FailedSocketCreation);

			if (connect(m_sock, m_result->ai_addr, (int)m_result->ai_addrlen) == SOCKET_ERROR)
			{
				closesocket(m_sock);
				return Response(FailedConnection);
			}

			m_ctx = SSL_CTX_new(SSLv23_client_method());
			m_ssl = SSL_new(m_ctx);
			SSL_set_fd(m_ssl, m_sock);
			if (SSL_connect(m_ssl) != 1)
			{
				SSL_free(m_ssl);
				closesocket(m_sock);
				SSL_CTX_free(m_ctx);
				return Response(FailedSSLConnection);
			}

			// Generate HTTP request
			std::stringstream ss;
			ss << type << " " << m_path << " HTTP/1.1\r\n";
			for (const auto& header : headers)
				ss << header.first << ": " << header.second << "\r\n";

			if (headers.find("Host") == headers.end())
				ss << "Host: " << m_host << "\r\n";
			if (headers.find("Connection") == headers.end())
				ss << "Connection: close\r\n";
			if (headers.find("Content-Length") == headers.end() && body != "")
				ss << "Content-Length: " << body.size() << "\r\n";

			ss << "\r\n";
			ss << body;

			if (SSL_write(m_ssl, ss.str().c_str(), ss.str().size()) <= 0)
			{
				SSL_free(m_ssl);
				closesocket(m_sock);
				SSL_CTX_free(m_ctx);
				return Response(FailedSendRequest);
			}

			std::string buffer;
			buffer.resize(1000);
			int bytesReceived = 0;
			int totalBytesReceived = 0;
			while (true)
			{
				bytesReceived = SSL_read(m_ssl, &buffer[0] + totalBytesReceived, 1000);
				if (bytesReceived <= 0)
				{
					if (totalBytesReceived == 0)
					{
						SSL_free(m_ssl);
						closesocket(m_sock);
						SSL_CTX_free(m_ctx);
						return Response(FailedReceiveResponse);
					}
					break;
				}
				totalBytesReceived += bytesReceived;
				buffer.resize(1000 + totalBytesReceived);
			}

			SSL_free(m_ssl);
			closesocket(m_sock);
			SSL_CTX_free(m_ctx);
			return Response(buffer);
		}
	
		Response get(std::unordered_map<std::string, std::string> headers = {}, std::string body = "")
		{
			return send("GET", headers, body);
		}

		Response post(std::unordered_map<std::string, std::string> headers = {}, std::string body = "")
		{
			return send("POST", headers, body);
		}
	private:
		std::string m_url;
		std::string m_host;
		std::string m_protocol;
		std::string m_port;
		std::string m_path;

		struct addrinfo* m_result = nullptr;

		SOCKET m_sock = INVALID_SOCKET;
		SSL_CTX* m_ctx = nullptr;
		SSL* m_ssl = nullptr;
	};
}
