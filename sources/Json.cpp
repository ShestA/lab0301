#include <stack>
#include <fstream>

#include "Json.h"
#include "JsonParser.h"

Json &Json::operator=(const Json &json)
{
    this->~Json();

    if (json.objectData) {
        this->objectData = new ObjectType(*json.objectData);
    }
    if (json.arrayData) {
        this->arrayData = new ArrayType(*json.arrayData);
    }

    return *this;
}

Json &Json::operator=(Json &&json) noexcept
{
    this->~Json();

    this->objectData = json.objectData;
    this->arrayData = json.arrayData;
    json.objectData = nullptr;
    json.arrayData = nullptr;

    return *this;
}

void Json::addToObjectKey(const std::string &key, const std::any &value)
{
    if (!objectData) {
        throw JsonUnexpectedType("Expected JSON object");
    }

    (*objectData)[key] = value;
}

void Json::addToArray(const std::any &value)
{
    if (!arrayData) {
        throw JsonUnexpectedType("Expected JSON array");
    }

    arrayData->push_back(value);
}

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

Json::Json(const std::string &string)
{
    *this = std::move(*JsonParser(string).result);
}

Json::~Json()
{
    if (objectData) {
        for (const auto &pair: *objectData) {
            if (pair.second.type() == typeid(Json *)) {
                delete std::any_cast<Json *>(pair.second);
            }
        }
    }

    if (arrayData) {
        for (const std::any &value: *arrayData) {
            if (value.type() == typeid(Json *)) {
                delete std::any_cast<Json *>(value);
            }
        }
    }

    delete objectData;
    delete arrayData;
}

std::any &Json::operator[](const std::string &key)
{
    if (!objectData) {
        throw JsonUnexpectedType("Expected JSON object");
    }

    if (objectData->find(key) == objectData->cend()) {
        throw JsonUnexpectedKey("Expected JSON object key: " + key);
    }

    return (*objectData)[key];
}

std::any &Json::operator[](int index)
{
    if (!arrayData) {
        throw JsonUnexpectedType("Expected JSON array");
    }

    if (arrayData->size() <= static_cast<size_t>(index)) {
        throw JsonUnexpectedKey("Expected JSON array index: " + std::to_string(index));
    }

    return (*arrayData)[index];
}

size_t Json::getSize() const
{
    if (arrayData) {
        return arrayData->size();
    } else if (objectData) {
        return objectData->size();
    } else {
        return 0;
    }
}

Json Json::parseFile(const std::string &path_to_file)
{
    std::ifstream fileStream(path_to_file);
    std::string fullFile(
        std::istreambuf_iterator<char>{fileStream},
        std::istreambuf_iterator<char>()
    );

    return Json(fullFile);
}
