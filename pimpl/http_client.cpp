#include "http_client.h"
#include <curl/curl.h>
#include <stdexcept>

struct HTTPClient::Impl {
    CURL* curl;

    Impl() {
        curl = curl_easy_init();
        if (!curl) throw std::runtime_error("Failed to initialize cURL.");
    }

    ~Impl() {
        curl_easy_cleanup(curl);
    }

    std::string perform(const std::string& url, const std::string* body = nullptr) {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        if (body) { // POST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body->c_str());
        } else { // GET
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }

        CURLcode result = curl_easy_perform(curl);
        if (result != CURLE_OK) throw std::runtime_error(curl_easy_strerror(result));

        return response;
    }

    static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
        auto* out = static_cast<std::string*>(userdata);
        out->append(ptr, size * nmemb);
        return size * nmemb;
    }
};

HTTPClient::HTTPClient() : pimpl(std::make_unique<Impl>()) {}

HTTPClient::~HTTPClient() = default;

HTTPClient::HTTPClient(HTTPClient&&) noexcept = default;
HTTPClient& HTTPClient::operator=(HTTPClient&&) noexcept = default;

std::string HTTPClient::get(const std::string& url) {
    return pimpl->perform(url);
}

std::string HTTPClient::post(const std::string& url, const std::string& body) {
    return pimpl->perform(url, &body);
}
