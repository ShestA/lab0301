#include "JsonParser.h"

JsonParser::JsonParser(const std::string &string) {
    for (char c : string) {
        currentChar = c;

        if (jsonEnded) {
            if (jsonEndedBehavior()) {
                continue;
            }
        }
        if (expectedDivider) {
            if (jsonExceptedDividerBehavior()) {
                continue;
            }
        }
        if (expectedObjectDivider) {
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

