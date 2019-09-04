#pragma once

#include <string>

namespace Utils
{

bool isCharSpace(char c);

bool isCharQuote(char c);

bool isCharNumber(char c);

double stringToNumber(const std::string &string);

}
