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

double Utils::stringToNumber(const std::string &string)
{
    return stod(string);
}
