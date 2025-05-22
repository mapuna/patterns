#include "request_builder.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

HttpRequest::HttpRequest(const std::string& url, HttpMethod method)
    : _url(url),
      _method(method),
      _connect_timeout(5000),
      _read_timeout(30000),
      _follow_redirects(true),
      _max_redirects(5),
      _verify_ssl(true) {}

std::string HttpRequest::build_full_url( ) const {
    if (_query_params.empty( )) return _url;

    std::ostringstream url_stream;
    url_stream << _url;

    bool first = true;
    for (const auto& [key, value] : _query_params) {
        url_stream << (first ? "?" : "&") << key << "=" << value;
        first = false;
    }

    return url_stream.str( );
}

std::string HttpRequest::get_method_string( ) const {
    switch (_method) {
        case HttpMethod::GET:
            return "GET";
        case HttpMethod::POST:
            return "POST";
        case HttpMethod::PUT:
            return "PUT";
        case HttpMethod::DELETE:
            return "DELETE";
        case HttpMethod::PATCH:
            return "PATCH";
        case HttpMethod::HEAD:
            return "HEAD";
        case HttpMethod::OPTIONS:
            return "OPTIONS";
        default:
            return "UNKNOWN";
    }
}

std::string HttpRequest::to_string( ) const {
    std::ostringstream ss;
    ss << get_method_string( ) << " " << build_full_url( ) << "\n";

    for (const auto& [key, value] : _headers)
        ss << key << ": " << value << "\n";

    if (!_body.empty( )) ss << "\n" << _body;

    return ss.str( );
}

HttpRequestBuilder::HttpRequestBuilder(const std::string& url,
                                       HttpMethod         method) {
    validate_url(url);
    _request = std::unique_ptr<HttpRequest>(new HttpRequest(url, method));
}

void HttpRequestBuilder::ensure_not_built( ) const {
    if (_built)
        throw std::runtime_error(
            "Request has already been built. Create a new builder for a new "
            "request");
}

void HttpRequestBuilder::validate_url(const std::string& url) const {
    if (url.empty( )) throw std::invalid_argument("URL cannot be empty");

    // this is a very basic validation -- needs improvements to handle all cases
    if (url.find("http://") != 0 && url.find("https://") != 0)
        throw std::invalid_argument("URL must start with http:// or https://");
}

HttpRequestBuilder& HttpRequestBuilder::add_header(const std::string& key,
                                                   const std::string& value) {
    ensure_not_built( );
    _request->_headers[key] = value;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::add_headers(const Headers& headers) {
    ensure_not_built( );
    for (const auto& [key, value] : headers) {
        _request->_headers[key] = value;
    }
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::add_query_param(
    const std::string& key, const std::string& value) {
    ensure_not_built( );
    _request->_query_params[key] = value;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::add_query_params(
    const QueryParams& params) {
    ensure_not_built( );
    for (const auto& [key, value] : params) {
        _request->_query_params[key] = value;
    }
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_body(const std::string& body) {
    ensure_not_built( );
    _request->_body = body;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_json_body(const std::string& json) {
    ensure_not_built( );
    _request->_body                    = json;
    _request->_headers["Content-Type"] = "application/json";
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_form_body(
    const QueryParams& form_data) {
    ensure_not_built( );

    std::ostringstream body_stream;
    bool               first = true;
    for (const auto& [key, value] : form_data) {
        body_stream << (first ? "" : "&") << key << "=" << value;
        first = false;
    }

    _request->_body                    = body_stream.str( );
    _request->_headers["Content-Type"] = "application/x-www-form-urlencoded";
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_connect_timeout(Timeout timeout) {
    ensure_not_built( );
    if (timeout.count( ) <= 0)
        throw std::invalid_argument("Timeout must be +ve");
    _request->_connect_timeout = timeout;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_read_timeout(Timeout timeout) {
    ensure_not_built( );
    if (timeout.count( ) <= 0)
        throw std::invalid_argument("Timeout must be +ve");
    _request->_read_timeout = timeout;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_basic_auth(
    const std::string& username, const std::string& password) {
    ensure_not_built( );
    _request->_auth_config.type     = AuthType::BASIC;
    _request->_auth_config.username = username;
    _request->_auth_config.password = password;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_bearer_token(
    const std::string& token) {
    ensure_not_built( );
    _request->_auth_config.type  = AuthType::BEARER;
    _request->_auth_config.token = token;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_api_key(
    const std::string& api_key, const std::string& header_name) {
    ensure_not_built( );
    _request->_auth_config.type           = AuthType::API_KEY;
    _request->_auth_config.api_key        = api_key;
    _request->_auth_config.api_key_header = header_name;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_follow_redirects(
    bool follow, int max_redirects) {
    ensure_not_built( );
    _request->_follow_redirects = follow;
    _request->_max_redirects    = max_redirects;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::set_verify_ssl(bool verify) {
    ensure_not_built( );
    _request->_verify_ssl = verify;
    return *this;
}

std::unique_ptr<HttpRequest> HttpRequestBuilder::build( ) {
    ensure_not_built( );
    apply_authentication( );
    validate_request( );

    _built = true;

    return std::move(_request);
}

void HttpRequestBuilder::apply_authentication( ) {
    switch (_request->_auth_config.type) {
        case AuthType::BASIC: {
            std::string auth_value = "Basic " +
                                     _request->_auth_config.username + ":" +
                                     _request->_auth_config.password;
            _request->_headers["Authorization"] = auth_value;
            break;
        }
        case AuthType::BEARER: {
            _request->_headers["Authorization"] =
                "Bearer " + _request->_auth_config.token;
            break;
        }
        case AuthType::API_KEY: {
            _request->_headers[_request->_auth_config.api_key_header] =
                _request->_auth_config.api_key;
            break;
        }
        case AuthType::NONE:
            break;
    }
}

void HttpRequestBuilder::validate_request( ) const {
    if (!_request->_body.empty( )) {
        switch (_request->_method) {
            case HttpMethod::POST:
            case HttpMethod::PUT:
            case HttpMethod::PATCH:
                if (_request->_headers.find("Content-Type") ==
                    _request->_headers.end( )) {
                    throw std::runtime_error(
                        "POST/PUT/PATCH requests with body must have "
                        "'Content-Type' header");
                }
                break;
            default:
                break;
        }
    }
}

std::unique_ptr<HttpRequest> HttpRequestDirector::build_json_api_request(
    const std::string& url, HttpMethod method, const std::string& json_body,
    const std::string& api_key) {
    HttpRequestBuilder builder(url, method);

    builder.add_header("Accept", "application/json")
        .add_header("User-Agent", "HttpClient/1.0");

    if (!json_body.empty( )) builder.set_json_body(json_body);

    if (!api_key.empty( )) builder.set_api_key(api_key);

    return builder.build( );
}

std::unique_ptr<HttpRequest> HttpRequestDirector::build_form_request(
    const std::string& url, const QueryParams& form_data) {
    return HttpRequestBuilder(url, HttpMethod::POST)
        .set_form_body(form_data)
        .add_header("User-Agent", "HttpClient/1.0")
        .build( );
}

std::unique_ptr<HttpRequest> HttpRequestDirector::build_download_request(
    const std::string& url, const std::string& auth_token) {
    HttpRequestBuilder builder(url, HttpMethod::GET);

    builder.set_read_timeout(std::chrono::minutes(10))
        .set_follow_redirects(true)
        .add_header("User-Agent", "HttpClient/1.0");

    if (!auth_token.empty( )) builder.set_bearer_token(auth_token);

    return builder.build( );
}
