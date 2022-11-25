#include <beauty/url.hpp>

#include <beauty/utils.hpp>

#include <iostream>
#if BEAUTY_USE_OLD_BOOST
#include <cstdlib>
#else
#include <charconv>
#endif

namespace {
// --------------------------------------------------------------------------
std::pair<std::string_view, std::string_view>
split_pair(const std::string_view& view, char sep, bool mandatory_left = true)
{
    std::pair<std::string_view, std::string_view> tmp;

    auto pair_split = beauty::split(view, sep);
    if (pair_split.size() == 1) {
        (mandatory_left ? tmp.first : tmp.second) = pair_split[0];
    } else if (pair_split.size() == 2) {
        tmp.first   = pair_split[0];
        tmp.second  = pair_split[1];
    } else {
        throw std::runtime_error("Invalid URL format for [" + std::string(view) + "]");
    }

    return tmp;
}
}

namespace beauty
{
// --------------------------------------------------------------------------
url::url(std::string u) : _url(std::move(u))
{
    //    0      1                 2 = host             3 = path
    // <scheme>://[[login][:password]@]<host>[:port][/path][?query]
    auto url_split = beauty::split(_url, '/');

    if (url_split.size() < 3 ||
            url_split[1].size() ||
            url_split[2].empty()) {
        throw std::runtime_error("Invalid URL format for [" + _url + "]");
    }

    // <scheme>:
    _scheme = url_split[0];
    if (!_scheme.empty() && *_scheme.rbegin() == ':') {
        _scheme.remove_suffix(1);
    }

    // [[login][:password]@]<host>[:port]
    auto [user_info, host] = split_pair(url_split[2], '@', false);
    if (user_info.size()) {
        std::tie(_login, _password) = split_pair(user_info, ':');
    }
    if (host.size()) {
        // IPv6
        if (host[0] == '[') {
            auto found = host.find("]");
            if (found == std::string::npos)
                throw std::runtime_error("Invalid URL format for IPv6 [" + std::string(host) + "]");

            _host = host.substr(1, found - 1);
            if ((found = host.find(':', found + 1)) != std::string::npos) {
                _port_view = host.substr(found + 1);
                if (_port_view.empty()) {
                    throw std::runtime_error("Invalid port for IPv6 [" + std::string(host) + "]");
                }
            }
        }
        else {
            std::tie(_host, _port_view) = split_pair(host, ':');
        }
    }
    if (_port_view.size()) {
#if BEAUTY_USE_OLD_BOOST
        char* end = const_cast<char*>(&_port_view[0] + _port_view.size());
        _port = std::strtoul(&_port_view[0], &end, 10);
        if (_port == 0) {
            throw std::runtime_error("Invalid port number " + std::string(_port_view));
        }
#else
        auto[p, ec] = std::from_chars(&_port_view[0],&_port_view[0] + _port_view.size(), _port);
        if (ec != std::errc()) {
            throw std::runtime_error("Invalid port number " + std::string(_port_view));
        }
#endif
    }

    // [/path][?query]
    if (url_split.size() >= 3) {
        // Compute the start of the path
        std::size_t pos = url_split[0].size() + 1
                + url_split[1].size() + 1
                + url_split[2].size();
        // Find the start of the query '?'
        auto found_query = _url.find('?', pos);
        if (found_query != std::string::npos) {
            _path = std::string_view(&_url[pos], found_query - pos);
            _query = std::string_view(&_url[found_query]);
        } else {
            _path = std::string_view(&_url[pos]);
        }
    }
}

// --------------------------------------------------------------------------
std::string
url::strip_login_password() const {
    return scheme() + "://" + host()
           + (port() ? ":" + std::to_string(port()) : "")
           + path();
}

}
