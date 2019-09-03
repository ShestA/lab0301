#pragma once

#include <stdexcept>

class JsonException : public std::runtime_error
{
public:
    explicit JsonException(const std::string &arg)
        : runtime_error(arg)
    {}

    explicit JsonException(const char *string)
        : runtime_error(string)
    {}
};