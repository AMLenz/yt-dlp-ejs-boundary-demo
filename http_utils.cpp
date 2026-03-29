#include "http_utils.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

#include "shared_cli_json.h"

namespace {
constexpr const char* DEFAULT_UA  = "Mozilla/5.0 (X11; Linux x86_64)";
constexpr const char* DEFAULT_ACC = "*/*";
}

int tcp_connect(const std::string& host, int port) {
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    const int rc = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (rc != 0) {
        throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(rc));
    }

    int fd = -1;
    for (auto p = res; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;

        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;

        close(fd);
        fd = -1;
    }

    freeaddrinfo(res);

    if (fd < 0) {
        throw std::runtime_error("connect failed to " + host);
    }

    return fd;
}

std::string decode_chunked(const std::string& body) {
    std::string out;
    size_t i = 0;

    while (i < body.size()) {
        const size_t e = body.find("\r\n", i);
        if (e == std::string::npos) break;

        std::string hex = trim(body.substr(i, e - i));
        const size_t semi = hex.find(';');
        if (semi != std::string::npos) {
            hex = hex.substr(0, semi);
        }

        unsigned long len = 0;
        try {
            len = std::stoul(hex, nullptr, 16);
        } catch (...) {
            break;
        }

        i = e + 2;

        if (len == 0 || i + len > body.size()) {
            break;
        }

        out.append(body, i, len);
        i += len;

        if (i + 2 <= body.size() && body.substr(i, 2) == "\r\n") {
            i += 2;
        }
    }

    return out;
}

HttpResponse https_request(const std::string& host,
                           const std::string& method,
                           const std::string& path,
                           const std::string& extraHeaders,
                           const std::string& body,
                           int redirectDepth) {
    if (redirectDepth > 8) {
        throw std::runtime_error("Too many redirects");
    }

    static bool ssl_init = false;
    if (!ssl_init) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        ssl_init = true;
    }

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        throw std::runtime_error("SSL_CTX_new failed");
    }

    SSL* ssl = nullptr;
    int fd = -1;

    try {
        fd = tcp_connect(host, 443);

        ssl = SSL_new(ctx);
        if (!ssl) {
            throw std::runtime_error("SSL_new failed");
        }

        SSL_set_fd(ssl, fd);
        SSL_set_tlsext_host_name(ssl, host.c_str());

        if (SSL_connect(ssl) != 1) {
            throw std::runtime_error("SSL_connect failed");
        }

        std::ostringstream req;
        req << method << " " << path << " HTTP/1.1\r\n"
            << "Host: " << host << "\r\n"
            << "Connection: close\r\n"
            << "User-Agent: " << DEFAULT_UA << "\r\n"
            << "Accept: " << DEFAULT_ACC << "\r\n";

        if (!body.empty()) {
            req << "Content-Type: application/json\r\n"
                << "Content-Length: " << body.size() << "\r\n";
        }

        if (!extraHeaders.empty()) {
            req << extraHeaders;
            if (extraHeaders.size() < 2 ||
                extraHeaders.substr(extraHeaders.size() - 2) != "\r\n") {
                req << "\r\n";
            }
        }

        req << "\r\n" << body;

        const std::string reqStr = req.str();
        std::string resp;

        for (size_t off = 0; off < reqStr.size();) {
            const int n = SSL_write(ssl, reqStr.data() + off,
                                    static_cast<int>(reqStr.size() - off));
            if (n <= 0) {
                throw std::runtime_error("SSL_write failed");
            }
            off += static_cast<size_t>(n);
        }

        char buf[16384];
        for (;;) {
            const int n = SSL_read(ssl, buf, sizeof(buf));
            if (n <= 0) break;
            resp.append(buf, buf + n);
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(fd);
        SSL_CTX_free(ctx);
        ssl = nullptr;
        fd = -1;
        ctx = nullptr;

        const size_t pos = resp.find("\r\n\r\n");
        if (pos == std::string::npos) {
            throw std::runtime_error("bad HTTP response");
        }

        HttpResponse r;
        r.headers = resp.substr(0, pos);
        r.body = resp.substr(pos + 4);

        {
            std::istringstream iss(r.headers);
            std::string httpver;
            iss >> httpver >> r.status;
        }

        std::string hl = r.headers;
        std::transform(hl.begin(), hl.end(), hl.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (hl.find("transfer-encoding: chunked") != std::string::npos) {
            r.body = decode_chunked(r.body);
        }

        if (r.status >= 300 && r.status < 400) {
            std::smatch m;
            if (std::regex_search(
                    r.headers,
                    m,
                    std::regex(R"((?:\r\n|^)Location:\s*([^\r\n]+))", std::regex::icase))) {
                const std::string loc = trim(m[1].str());

                if (loc.rfind("https://", 0) == 0) {
                    std::smatch lm;
                    if (std::regex_match(loc, lm, std::regex(R"(^https://([^/]+)(/.*)?$)"))) {
                        return https_request(
                            lm[1].str(),
                            method,
                            lm[2].matched ? lm[2].str() : "/",
                            extraHeaders,
                            body,
                            redirectDepth + 1
                        );
                    }
                } else if (!loc.empty() && loc[0] == '/') {
                    return https_request(host, method, loc, extraHeaders, body, redirectDepth + 1);
                }
            }
        }

        return r;
    } catch (...) {
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        if (fd >= 0) close(fd);
        if (ctx) SSL_CTX_free(ctx);
        throw;
    }
}
