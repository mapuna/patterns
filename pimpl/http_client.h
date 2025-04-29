#ifndef HTTP_CLIENT
#define HTTP_CLIENT

#include <memory>
#include <string>

class HTTPClient {
public:
    HTTPClient();
    ~HTTPClient();

    // no copying
    HTTPClient(const HTTPClient&) = delete;
    HTTPClient& operator=(const HTTPClient&) = delete;

    // move allowed
    HTTPClient(HTTPClient&&) noexcept;
    HTTPClient& operator=(HTTPClient&&) noexcept;

    std::string get(const std::string& url);
    std::string post(const std::string& url, const std::string& body);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif // HTTP_CLIENT
