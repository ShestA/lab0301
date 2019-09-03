#pragma once

#include <stack>
#include <functional>
#include "Utils.h"
#include "Json.h"

class JsonParser
{
public:
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

    explicit JsonParser(const std::string &string);

    [[nodiscard]] const Json &getResult() const
    {
        return *result;
    }

    virtual ~JsonParser()
    {
        delete result;
    }

    friend Json;

private:
    static void jsonConvertPointers(Json *json)
    {
        if (json->is_array()) {
            for (size_t i = 0; i < json->getSize(); i++) {
                if (std::any &value = (*json)[i]; value.type() == typeid(Json *)) {
                    Json &nested = *std::any_cast<Json *>(value);
                    (*json)[i] = std::move(nested);
                }
            }
        } else if (json->is_object()) {
            for (const std::string &key : json->getKeys()) {
                try {
                    Json &nested = *std::any_cast<Json *>((*json)[key]);
                    (*json)[key] = std::move(nested);
                } catch (std::bad_any_cast &) {

                }
            }
        }
    }

    void finishCurrentJson()
    {
        jsonConvertPointers(jsonStack.top());

        jsonStack.pop();
        currentType = NoneParsing;

        if (jsonStack.empty()) {
            jsonEnded = true;
        } else {
            if (jsonStack.top()->is_object()) {
                isObjectKey = true;
            }

            expectedDivider = true;
        }
    }

    bool jsonEndedBehavior()
    {
        if (!Utils::isCharSpace(currentChar)) {
            throw JsonException("");
        }

        return true;
    }

    bool jsonExceptedDividerBehavior()
    {
        if (currentChar == ',') {
            expectedDivider = false;
        } else if ((currentChar == '}' && jsonStack.top()->is_object())
            || (currentChar == ']' && jsonStack.top()->is_array())) {

            finishCurrentJson();
        } else if (!Utils::isCharSpace(currentChar)) {
            throw JsonException(std::string("Expected ',', got '") + currentChar + "'");
        }

        return true;
    }

    bool jsonExceptedObjectDividerBehavior()
    {
        if (currentChar == ':') {
            expectedObjectDivider = false;
        } else if (!Utils::isCharSpace(currentChar)) {
            throw JsonException(std::string("Expected ':', got '") + currentChar + "'");
        }

        return true;
    }

    Json *createNewJsonIfCan()
    {
        if (currentChar == '{') {
            // Инициализация JSON объекта
            isObjectKey = true;
            jsonStack.push(new Json{Json::ObjectType{}});
        } else if (currentChar == '[') {
            // Инициализация JSON массива
            jsonStack.push(new Json{Json::ArrayType{}});
        } else {
            // Не смог
            return nullptr;
        }

        return jsonStack.top();
    }

    bool jsonNoJsonBehavior()
    {
        if (Utils::isCharSpace(currentChar)) {
            return true;            // ok, continue
        }

        Json *created = createNewJsonIfCan();
        if (!created) {
            throw JsonException("Expected begin of object or array");
        }
        result = created;

        return false;
    }

    bool parseNewValueIfCan()
    {
        if (Utils::isCharQuote(currentChar)) {
            // Начало строки
            currentType = StringValue;
            stringOpenQuote = currentChar;
            currentValue.clear();
        } else if (Utils::isCharNumber(currentChar)) {
            // Начало числа
            currentType = Number;
            currentValue = currentChar;
        } else if (currentChar == 't' || currentChar == 'f') {
            // Начало bool значения
            currentType = Boolean;
            currentValue = currentChar;
        } else {
            return false;
        }

        return true;
    }

    void clearValue(bool willBeExpectedDivider)
    {
        stringOpenQuote = '\0';
        currentType = NoneParsing;
        expectedDivider = willBeExpectedDivider;
    }

    bool continueParsingValueIfCan(const std::function<void(const std::any &)> &addFunction)
    {
        if (currentType == StringValue) {
            if (currentChar == stringOpenQuote) {
                addFunction(currentValue);
                clearValue(true);
            } else {
                currentValue += currentChar;
            }
        } else if (currentType == Number) {
            if (Utils::isCharSpace(currentChar) || currentChar == ',') {
                double number = stod(currentValue);

                addFunction(number);
                clearValue(currentChar != ',');
            } else if ((currentChar == '}' && jsonStack.top()->is_object())
                || (currentChar == ']' && jsonStack.top()->is_array())) {

                double number = stod(currentValue);

                addFunction(number);
                finishCurrentJson();
            } else {
                currentValue += currentChar;
            }
        } else if (currentType == Boolean) {
            if ((currentValue == "tru" || currentValue == "fals") && currentChar == 'e') {
                bool value = currentValue == "tru";

                addFunction(value);
                clearValue(true);
            } else if (Utils::isCharSpace(currentChar) || currentChar == ',') {
                throw JsonException("");
            } else {
                currentValue += currentChar;
            }
        } else {
            return false;
        }

        return true;
    }

    bool jsonFillArrayBehavior()
    {
        if (currentType == NoneParsing) {
            if (Utils::isCharSpace(currentChar)) {
                return true;            // continue
            }
            if (parseNewValueIfCan()) {
                return true;            // ok, continue
            }
            if (Json *prevContainer = this->jsonStack.top(), *created = createNewJsonIfCan(); created) {
                prevContainer->addToArray(created);
                return true;            // ok, continue
            }
            if (currentChar == ']') {
                finishCurrentJson();
                return true;
            }

            throw JsonException("");        // something undefined
        }

        bool parsingResult = continueParsingValueIfCan(
            [this](const std::any &value) {
                Json &json = *this->jsonStack.top();
                json.addToArray(value);
            }
        );

        if (parsingResult) {
            return true;            // ok, continue
        }

        throw JsonException("Internal error");
    }

    bool jsonFillObjectBehavior()
    {
        if (isObjectKey) {
            if (currentType == StringKey) {
                if (currentChar == stringOpenQuote) {
                    jsonStack.top()->addToObjectKey(currentValue, {});
                    currentKey = currentValue;

                    clearValue(false);
                    isObjectKey = false;
                    expectedObjectDivider = true;
                    return true;
                }

                currentValue += currentChar;
            } else if (currentType == NoneParsing) {
                if (Utils::isCharSpace(currentChar)) {
                    return true;
                }
                if (currentChar == '}') {
                    finishCurrentJson();
                    return true;
                }

                if (Utils::isCharQuote(currentChar)) {
                    stringOpenQuote = currentChar;
                    currentType = StringKey;
                    currentValue.clear();
                } else {
                    throw JsonException("Expected quote");
                }
            } else {
                throw JsonException("Internal error");
            }

            return true;
        }

        if (currentType == NoneParsing) {
            if (Utils::isCharSpace(currentChar)) {
                return true;            // continue
            }
            if (parseNewValueIfCan()) {
                return true;            // ok, continue
            }
            if (Json *prevContainer = this->jsonStack.top(), *created = createNewJsonIfCan(); created) {
                prevContainer->addToObjectKey(currentKey, created);
                return true;            // ok, continue
            }
            if (currentChar == '}') {
                finishCurrentJson();
                return true;
            }

            throw JsonException("");        // something undefined
        }

        bool parsingResult = continueParsingValueIfCan(
            [this](const std::any &value) {
                Json &json = *this->jsonStack.top();
                json.addToObjectKey(currentKey, value);
            }
        );

        if (parsingResult) {
            if (currentType == NoneParsing) {
                isObjectKey = true;
                currentKey.clear();
            }
            return true;
        }

        throw JsonException("Internal error");
    }

    ParsingType currentType = NoneParsing;          // Тип, который парсим в данный момент
    bool isObjectKey = false;                       // Является ли текущий парсинг - парсингом ключа объекта
    std::string currentKey;                         // Текущий ключ
    bool expectedDivider = false;                   // Следующий ожидаемый символ - разделитель?
    bool expectedObjectDivider = false;             // Следующий ожидаемый символ -разделитель между ключом и значением?
    bool jsonEnded = false;                         // Закончилась ли JSON строка (по смыслу)

    std::stack<Json *> jsonStack;                   // Стэк из JSON множеств (нужен для вложенности)

    char currentChar;                               // Текущий символ
    std::string currentValue;                       // Текущее считываемое значение
    char stringOpenQuote = '\0';                    // Ковычка, с которой началась строка

    Json *result = nullptr;                         // Результат
};