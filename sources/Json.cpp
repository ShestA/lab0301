#include <stack>

#include "Json.h"
#include "JsonParser.h"
#include "Utils.h"

bool Json::is_array() const
{
    if (Json::is_null()) {
        return false;
    }

    return arrayData != nullptr;
}

bool Json::is_object() const
{
    if (Json::is_null()) {
        return false;
    }

    return objectData != nullptr;
}

bool Json::is_null() const
{
    return objectData == nullptr && arrayData == nullptr;
}

Json Json::parse(const std::string &)
{
    return Json();          // TODO
}

Json::Json(const std::string &string)
{
    *this = std::move(*JsonParser(string).result);
}

Json::~Json()
{
    // TODO: обходить все данные и вызывать деструкторы у json

    delete objectData;
    delete arrayData;
}

std::any &Json::operator[](const std::string &key)
{
    if (!objectData) {
        throw JsonException("");
    }

    if (objectData->find(key) == objectData->cend()) {
        throw JsonException("");
    }

    return (*objectData)[key];
}

std::any &Json::operator[](int index)
{
    if (!arrayData) {
        throw JsonException("");
    }

    if (arrayData->size() <= static_cast<size_t>(index)) {
        throw JsonException("");
    }

    return (*arrayData)[index];
}
