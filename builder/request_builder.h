#ifndef REQUEST_BUILDER_H
#define REQUEST_BUILDER_H

#include <chrono>
#include <unordered_map>

class HttpRequest;
class HttpRequestBuilder;

using Headers     = std::unordered_map<std::string, std::string>;
using QueryParams = std::unordered_map<std::string, std::string>;
using Timeout     = std::chrono::milliseconds;

enum class HttpMethod { GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS };

enum class AuthType { NONE, BASIC, BEARER, API_KEY };

struct AuthConfig {
    AuthType    type = AuthType::NONE;
    std::string username;
    std::string password;
    std::string token;
    std::string api_key;
    std::string api_key_header = "X-API-Key";
};

/**
 * The request
 */
class HttpRequest {
    std::string _url;
    HttpMethod  _method;
    Headers     _headers;
    QueryParams _query_params;
    std::string _body;
    Timeout     _connect_timeout;
    Timeout     _read_timeout;
    AuthConfig  _auth_config;
    bool        _follow_redirects;
    int         _max_redirects;
    bool        _verify_ssl;

    friend class HttpRequestBuilder;

    HttpRequest(const std::string& url, HttpMethod method);

public:
    // simple getters -- may not need them
    const std::string& get_url( ) const { return _url; }
    HttpMethod         get_method( ) const { return _method; }
    const Headers&     get_headers( ) const { return _headers; }
    const QueryParams& get_query_params( ) const { return _query_params; }
    const std::string& get_body( ) const { return _body; }
    Timeout            get_connect_timeout( ) const { return _connect_timeout; }
    Timeout            get_read_timeout( ) const { return _read_timeout; }
    const AuthConfig&  get_auth_config( ) const { return _auth_config; }
    bool should_follow_redirects( ) const { return _follow_redirects; }
    int  get_max_redirects( ) const { return _max_redirects; }
    bool should_verify_ssl( ) const { return _verify_ssl; }

    // builds the final URL with query params
    std::string build_full_url( ) const;

    // for logging / debugging
    std::string get_method_string( ) const;

    // Python __str__ kind of method to retireve the entire request
    std::string to_string( ) const;
};

/**
 * The builder
 */
class HttpRequestBuilder {
    std::unique_ptr<HttpRequest> _request;
    bool                         _built = false;

    void ensure_not_built( ) const;
    void validate_url(const std::string& url) const;
    void apply_authentication( );
    void validate_request( ) const;

public:
    HttpRequestBuilder(const std::string& url, HttpMethod method);

    // **fluent** interface methods for building the request
    HttpRequestBuilder& add_header(const std::string& key,
                                   const std::string& value);
    HttpRequestBuilder& add_headers(const Headers& headers);
    HttpRequestBuilder& add_query_param(const std::string& key,
                                        const std::string& value);
    HttpRequestBuilder& add_query_params(const QueryParams& params);
    HttpRequestBuilder& set_body(const std::string& body);
    HttpRequestBuilder& set_json_body(const std::string& json);
    HttpRequestBuilder& set_form_body(const QueryParams& form_data);
    HttpRequestBuilder& set_connect_timeout(Timeout timeout);
    HttpRequestBuilder& set_read_timeout(Timeout timeout);
    HttpRequestBuilder& set_basic_auth(const std::string& username,
                                       const std::string& password);
    HttpRequestBuilder& set_bearer_token(const std::string& token);
    HttpRequestBuilder& set_api_key(
        const std::string& api_key,
        const std::string& header_name = "X-API-Key");
    HttpRequestBuilder& set_follow_redirects(bool follow,
                                             int  max_redirects = 5);
    HttpRequestBuilder& set_verify_ssl(bool verify);

    /**
     * final request is built here
     */
    std::unique_ptr<HttpRequest> build( );
};

/**
 * The Director -- optional extension to the original builder pattern
 *
 * Director knows how to build specific types of requests using the builder.
 * Automatically handles "typycal" request patterns
 */
class HttpRequestDirector {
public:
    /**
     * build a "standard" JSON API request
     */
    static std::unique_ptr<HttpRequest> build_json_api_request(
        const std::string& url, HttpMethod method,
        const std::string& json_body = "", const std::string& api_key = "");

    /**
     * build a form submit request
     */
    static std::unique_ptr<HttpRequest> build_form_request(
        const std::string& url, const QueryParams& form_data);

    /**
     * build a file download request
     */
    static std::unique_ptr<HttpRequest> build_download_request(
        const std::string& url, const std::string& auth_token = "");
};

#endif  // REQUEST_BUILDER_H
