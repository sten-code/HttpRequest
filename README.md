# HTTP Library

This is a library used to make http/https requests in C++

## Usage

```cpp
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
```

Go into `examples` for more examples like these.

### Windows

You need to have openssl installed which you can get [here](https://wiki.openssl.org/index.php/Binaries).

- Include directory: `C:\Program Files\OpenSSL-Win64\include`
- Library directory: `C:\Program Files\OpenSSL-Win64\lib`
- Required libraries: `libssl.lib` and `libcrypto.lib`

### Linux

You need to compile openssl yourself, which you can get [here](https://github.com/openssl/openssl/releases).

The makefile should automatically do this when compiling the examples

```bash
wget https://github.com/openssl/openssl/releases/download/openssl-3.2.1/openssl-3.2.1.tar.gz
tar -xf openssl-*.tar.gz
rm openssl-*.tar.gz
mv openssl-* openssl
cd openssl
./Configure
make
```

- Include directory: `-Iopenssl/include`
- Library directory: `-Lopenssl`
- Required libraries: `-l:libssl.a` and `-l:libcrypto.a`
