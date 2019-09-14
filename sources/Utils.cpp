#include <cstring>
#include <JsonException.h>
#include "Utils.h"

bool Utils::isCharSpace(char c)
{
    return c == ' '
        || c == '\t'
        || c == '\r'
        || c == '\n';
}

bool Utils::isCharQuote(char c)
{
    return c == '\''
        || c == '\"';
}

bool Utils::isCharNumber(char c)
{
    return c >= '0' && c <= '9';
}

bool Utils::isCharEscaping(char c)
{
    return c == '\\';
}

double Utils::stringToNumber(const std::string &string)
{
    char *unexpectedChars;
    double value = std::strtod(string.c_str(), &unexpectedChars);
    if (std::strlen(unexpectedChars) > 0) {
        throw JsonParseCannotParseNumber("Cannot convert '" + string + "' to number");
    }

    return value;
}
