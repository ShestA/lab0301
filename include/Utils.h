#pragma once

namespace Utils
{

bool isCharSpace(char c)
{
    return c == ' '
        || c == '\t'
        || c == '\r'
        || c == '\n';
}

bool isCharQuote(char c)
{
    return c == '\''
        || c == '\"';
}

bool isCharNumber(char c)
{
    return c >= '0' && c <= '9';
}

bool isCharDivider(char c)
{
    return c == ',';
}

}
