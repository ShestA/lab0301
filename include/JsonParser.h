#pragma once

#include <stack>
#include <functional>
#include "Json.h"

class JsonParser
{
public:
    // Типы, возможные для сериализации (исключая контейнерные)
    enum ParsingType
    {
        StringKey,
        StringValue,
        Number,
        Boolean,
        NoneParsing,
    };

    // Конструктор от строки
    explicit JsonParser(const std::string &string);

    // Геттер для результата
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
    // Завершить верхний по стеку json объект.
    // Если стек будет пусть - установить полное завершение сериализации
    // При наличии в стеке json-объекта или json-массива - выставить необходимые флаги
    void finishCurrentJson();

    // Поведение при гарантировано закончившимся JSON'е
    // Вернет true, если необходимо перейти к следующий итерации основного цикла. Иначе - false
    bool jsonEndedBehavior();

    // Поведение при ожидании разделителя ','
    // Вернет true, если необходимо перейти к следующий итерации основного цикла. Иначе - false
    bool jsonExceptedDividerBehavior();

    // Поведение при ожидании раздалителя ключа от значения ':'
    // Вернет true, если необходимо перейти к следующий итерации основного цикла. Иначе - false
    bool jsonExceptedObjectDividerBehavior();

    // Создать новый контейнер, если это возможно
    // Вернет указатель на созданный (в куче) JSON, если удалось. Иначе - nullptr
    Json *createNewJsonIfCan();

    // Поведение при отсутствии контейнеров
    // Вернет true, если необходимо перейти к следующий итерации основного цикла. Иначе - false
    bool jsonNoJsonBehavior();

    // Начать сериализацию нового значения, если возможно
    // Вернет true, если удалось. Иначе - false
    bool parseNewValueIfCan();

    // Очистить текущее считываемое значение
    void clearValue(bool willBeExpectedDivider);

    // Продолжить считывание текущего значения, если возможно
    // Вернет true, если удалось. Иначе - false
    bool continueParsingValueIfCan(const std::function<void(const std::any &)> &addFunction);

    // Поведение при наличии json-массива на вершине стека
    // Вернет true, если необходимо перейти к следующий итерации основного цикла. Иначе - false
    bool jsonFillArrayBehavior();

    // Поведение при наличии json-объекта на вершине стека
    // Вернет true, если необходимо перейти к следующий итерации основного цикла. Иначе - false
    bool jsonFillObjectBehavior();

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