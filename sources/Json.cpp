#include <stack>

#include "Json.h"

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

Json::Json(const std::string &s)
{
    enum ContainerType
    {
        Object,
        Array,
        NoneContainer,
    };

    enum ParsingType
    {
        StringKey,
        StringValue,
        ObjectKey,
        Number,
        Boolean,
        NoneParsing,
    };

    ParsingType currentType = NoneParsing;          // Тип, который парсим в данный момент
    bool isObjectKey = false;                // Является ли текущий парсинг - парсингом ключа
    bool expectedDivider = false;
    bool jsonEnded = false;

    std::stack<Json *> jsonStack;

    std::string currentValue;
    char stringOpenQuote = '\0';

    for (char c : s) {
        if (jsonEnded) {
            if (!Utils::isCharSpace(c)) {
                throw JsonException("");
            }
            continue;
        }

        if (expectedDivider) {
            if (c == ',') {
                expectedDivider = false;
            } else if ((c == '}' && jsonStack.top()->is_object()) || (c == ']' && jsonStack.top()->is_array())) {
                jsonStack.pop();
                if (jsonStack.empty()) {
                    jsonEnded = true;
                } else {
                    expectedDivider = true;
                }
            } else if (!Utils::isCharSpace(c)) {
                throw JsonException(std::string("Expected ',', got '") + c + "'");
            }
            continue;
        }

        if (jsonStack.empty()) {
            // Некуда класть значения. Ожидаем множество (объект или массив)
            if (c == '{') {
                objectData = new ObjectType{};
                isObjectKey = true;
                jsonStack.push(this);
            } else if (c == '[') {
                arrayData = new ArrayType{};
                jsonStack.push(this);
            } else if (!Utils::isCharSpace(c)) {
                throw JsonException("Expected begin of object or array");
            }
        } else if (jsonStack.top()->is_array()) {
            if (currentType == NoneParsing) {
                if (!Utils::isCharSpace(c)) {
                    if (Utils::isCharQuote(c)) {
                        // Начало строки
                        currentType = StringValue;
                        stringOpenQuote = c;
                        currentValue.clear();
                    } else if (Utils::isCharNumber(c)) {
                        // Начало числа
                        currentType = Number;
                        currentValue = c;
                    } else if (c == 't' || c == 'f') {
                        // Начало bool значения
                        currentType = Boolean;
                        currentValue = c;
                    } else if (c == ']') {
                        jsonStack.pop();
                        if (jsonStack.empty()) {
                            jsonEnded = true;
                        } else {
                            expectedDivider = true;
                        }
                    } else {
                        throw JsonException("");
                    }
                }
            } else if (currentType == StringValue) {
                if (c == stringOpenQuote) {
                    Json &json = *jsonStack.top();
                    json.addToArray(currentValue);

                    stringOpenQuote = '\0';
                    currentType = NoneParsing;
                    expectedDivider = true;
                } else {
                    currentValue += c;
                }
            } else if (currentType == Number) {
                if (Utils::isCharSpace(c) || c == ',') {
                    double number = stod(currentValue);

                    Json &json = *jsonStack.top();
                    json.addToArray(number);

                    currentType = NoneParsing;

                    if (c != ',') {
                        expectedDivider = true;
                    }
                } else {
                    currentValue += c;
                }
            } else if (currentType == Boolean) {
                if ((currentValue == "tru" || currentValue == "fals") && c == 'e') {
                    bool value = currentValue == "true";

                    Json &json = *jsonStack.top();
                    json.addToArray(value);

                    currentType = NoneParsing;
                    expectedDivider = true;
                } else if (Utils::isCharSpace(c) || c == ',') {
                    throw JsonException("");
                } else {
                    currentValue += c;
                }
            } else {
                throw JsonException("Internal error");
            }
        } else if (jsonStack.top()->is_object()) {
            if (currentType == NoneParsing) {
                if (c == '}') {
                    jsonStack.pop();
                    expectedDivider = true;
                    continue;
                }
            }

            if (isObjectKey) {
                if (currentType == StringKey) {
                    if (c == stringOpenQuote) {
                        jsonStack.top()->addObjectKey(currentValue);

                        stringOpenQuote = '\0';
                        currentType = NoneParsing;
                        expectedDivider = true;
                    }
                    currentValue += c;
                } else if (currentType == NoneParsing) {
                    if (!Utils::isCharSpace(c)) {
                        if (Utils::isCharQuote(c)) {
                            stringOpenQuote = c;
                            currentType = StringKey;
                            currentValue.clear();
                        } else {
                            throw JsonException("Expected quote");
                        }
                    }
                } else {
                    throw JsonException("Internal error");
                }
            } else {
                // TODO
            }
        }
    }

    if (!jsonStack.empty() || currentType != NoneParsing) {
        throw JsonException("Unexpected end of string");
    }
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
