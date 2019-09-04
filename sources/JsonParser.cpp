#include "JsonParser.h"
#include "Utils.h"

JsonParser::JsonParser(const std::string &string) {
    for (char now : string) {       // Основной цикл
        currentChar = now;

        if (jsonEnded) {
            // JSON гарантировано закончился
            if (jsonEndedBehavior()) {
                continue;
            }
        }

        if (expectedDivider) {
            // Ожидание ,
            if (jsonExceptedDividerBehavior()) {
                continue;
            }
        }

        if (expectedObjectDivider) {
            // Ожидание :
            if (jsonExceptedObjectDividerBehavior()) {
                continue;
            }
        }

        if (jsonStack.empty()) {
            // Некуда класть значения. Ожидаем множество (объект или массив)
            if (jsonNoJsonBehavior()) {
                continue;
            }
        } else if (jsonStack.top()->is_array()) {
            // Есть массив
            if (jsonFillArrayBehavior()) {
                continue;
            }
        } else if (jsonStack.top()->is_object()) {
            // Есть объект
            if (jsonFillObjectBehavior()) {
                continue;
            }
        }
    }

    if (!jsonEnded) {
        throw JsonException("Unexpected end of string");
    }
}

void JsonParser::finishCurrentJson()
{
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

bool JsonParser::jsonEndedBehavior()
{
    if (!Utils::isCharSpace(currentChar)) {
        throw JsonException("");
    }

    return true;
}

bool JsonParser::jsonExceptedDividerBehavior()
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

bool JsonParser::jsonExceptedObjectDividerBehavior()
{
    if (currentChar == ':') {
        expectedObjectDivider = false;
    } else if (!Utils::isCharSpace(currentChar)) {
        throw JsonException(std::string("Expected ':', got '") + currentChar + "'");
    }

    return true;
}

Json *JsonParser::createNewJsonIfCan()
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

bool JsonParser::jsonNoJsonBehavior()
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

bool JsonParser::parseNewValueIfCan()
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

void JsonParser::clearValue(bool willBeExpectedDivider)
{
    stringOpenQuote = '\0';
    currentType = NoneParsing;
    expectedDivider = willBeExpectedDivider;
}

bool JsonParser::continueParsingValueIfCan(const std::function<void(const std::any &)> &addFunction)
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
            double number = Utils::stringToNumber(currentValue);

            addFunction(number);
            clearValue(currentChar != ',');
        } else if ((currentChar == '}' && jsonStack.top()->is_object())
            || (currentChar == ']' && jsonStack.top()->is_array())) {

            double number = Utils::stringToNumber(currentValue);

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

bool JsonParser::jsonFillArrayBehavior()
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

bool JsonParser::jsonFillObjectBehavior()
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
