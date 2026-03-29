#pragma once

#include <string>

#include "yoube_model.h"

int tcp_connect(const std::string& host, int port);

std::string decode_chunked(const std::string& body);

HttpResponse https_request(const std::string& host,
                           const std::string& method,
                           const std::string& path,
                           const std::string& extraHeaders = "",
                           const std::string& body = "",
                           int redirectDepth = 0);
