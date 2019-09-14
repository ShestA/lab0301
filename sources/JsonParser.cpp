#include "JsonParser.h"
#include "Utils.h"

JsonParser::JsonParser(const std::string &string)
{
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
        throw JsonParseUnexpectedEof("Unexpected end of json data");
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
        throw JsonParseUnexpectedChar("Expected end of json data");
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
        throw JsonParseUnexpectedChar(std::string("Expected ',', got '") + currentChar + "'");
    }

    return true;
}

bool JsonParser::jsonExceptedObjectDividerBehavior()
{
    if (currentChar == ':') {
        expectedObjectDivider = false;
    } else if (!Utils::isCharSpace(currentChar)) {
        throw JsonParseUnexpectedChar(std::string("Expected ':', got '") + currentChar + "'");
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
        throw JsonParseUnexpectedChar(std::string("Expected begin of object or array, got '") + currentChar + "'");
    }
    result.reset(created);

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
    } else if (currentChar == 't' || currentChar == 'f' || currentChar == 'n') {
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
    isEscapeChar = false;
    stringOpenQuote = '\0';
    currentType = NoneParsing;
    expectedDivider = willBeExpectedDivider;
}

bool JsonParser::continueParsingValueIfCan(const std::function<void(const std::any &)> &addFunction)
{
    if (currentType == StringValue) {
        if (currentChar == stringOpenQuote && !isEscapeChar) {
            // Завершение чтения строки
            addFunction(currentValue);
            clearValue(true);
        } else if (Utils::isCharEscaping(currentChar)) {
            isEscapeChar = true;
        } else {
            // Сбросить значение экранирования
            if (isEscapeChar) {
                isEscapeChar = false;
            }

            // Продолжение чтения строки
            currentValue += currentChar;
        }
    } else if (currentType == Number) {
        if (Utils::isCharSpace(currentChar) || currentChar == ',') {
            // Завершение чтения числа
            double number = Utils::stringToNumber(currentValue);

            addFunction(number);
            clearValue(currentChar != ',');
        } else if ((currentChar == '}' && jsonStack.top()->is_object())
            || (currentChar == ']' && jsonStack.top()->is_array())) {
            // Завершение чтения числа, причем вместе с завершением чтения контейнера
            double number = Utils::stringToNumber(currentValue);

            addFunction(number);
            finishCurrentJson();
        } else {
            // Продолжение чтения числа
            currentValue += currentChar;
        }
    } else if (currentType == Boolean) {
        if ((currentValue == "tru" || currentValue == "fals") && currentChar == 'e') {
            // Завершение чтения ключевого слова
            bool value = currentValue == "tru";

            addFunction(value);
            clearValue(true);
        } else if (currentValue == "nul" && currentChar == 'l') {
            // Завершение чтения ключевого слова
            addFunction({});
            clearValue(true);
        } else if (Utils::isCharSpace(currentChar) || currentChar == ',') {
            // Завершение чтения ключевого слова, однако ключевое слово не распознано
            throw JsonParseUnexpectedChar(
                std::string("Unexpected keyword. Expected: true, false, null. Got: ") + currentValue
            );
        } else {
            // Продолжение чтения ключевого слова
            currentValue += currentChar;
        }
    } else {
        // Не смог продолжить читать
        return false;
    }

    return true;        // ok, continue
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

        throw JsonParseInternalError(std::string("Attempt to fill array with unexpected character ") + currentChar);
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

    throw JsonParseInternalError("Unexpected error while filling array");
}

bool JsonParser::jsonFillObjectBehavior()
{
    if (isObjectKey) {
        // Ожидание ключа
        if (currentType == StringKey) {
            // Уже читаем ключ
            if (currentChar == stringOpenQuote && !isEscapeChar) {
                // Закончить читать ключ

                if (const auto &keys = jsonStack.top()->getKeys();
                    std::find(keys.cbegin(), keys.cend(), currentValue) != keys.cend()) {
                    // Такой ключ уже существует
                    throw JsonParseDuplicatedKeyError("Key '" + currentValue + "' already exists");
                }

                jsonStack.top()->addToObjectKey(currentValue, {});
                currentKey = currentValue;

                clearValue(false);
                isObjectKey = false;
                expectedObjectDivider = true;
                return true;
            } else if (Utils::isCharEscaping(currentChar)) {
                isEscapeChar = true;
                return true;
            }

            // Сбросить экранирование
            if (isEscapeChar) {
                isEscapeChar = false;
            }

            // Продолжить чтение ключа
            currentValue += currentChar;
        } else if (currentType == NoneParsing) {
            // Начать сериализацию ключа

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
                throw JsonParseUnexpectedChar(std::string("Expected quote, got '") + currentChar + "'");
            }
        } else {
            throw JsonParseInternalError("Expected string at key");
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

        throw JsonParseUnexpectedChar(std::string("Expected value after key. Got: ") + currentChar);
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

    throw JsonParseInternalError("Unexpected error while filling object");
}
