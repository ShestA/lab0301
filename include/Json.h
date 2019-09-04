#pragma once

#include <string>
#include <any>
#include <unordered_map>
#include <vector>

#include "JsonException.h"

class Json
{
public:
    using ObjectType = std::unordered_map<std::string, std::any>;
    using ArrayType = std::vector<std::any>;

    // Конструктор из строки, содержащей Json-данные.
    explicit Json(const std::string &string);

    explicit Json(const ObjectType& object):
        objectData(new ObjectType(object))
    {}

    explicit Json(const ArrayType& object):
        arrayData(new ArrayType(object))
    {}

    // Пустой конструктор
    Json() = default;

    Json(const Json &json)
    {
        *this = json;
    }

    Json(Json &&json) noexcept
    {
        *this = std::forward<Json>(json);
    }

    Json &operator=(const Json &json)
    {
        this->~Json();

        if (json.objectData) {
            this->objectData = new ObjectType{*json.objectData};
        }
        if (json.arrayData) {
            this->arrayData = new ArrayType{*json.arrayData};
        }

        return *this;
    }

    Json &operator=(Json &&json) noexcept
    {
        this->~Json();

        this->objectData = json.objectData;
        this->arrayData = json.arrayData;
        json.objectData = nullptr;
        json.arrayData = nullptr;

        return *this;
    }

    // Добавить чето в словарик
    void addToObjectKey(const std::string &key, const std::any &value)
    {
        if (!objectData) {
            throw JsonException("");
        }

        (*objectData)[key] = value;
    }

    // Добавить чето в массив
    void addToArray(const std::any &value)
    {
        if (!arrayData) {
            throw JsonException("");
        }

        arrayData->push_back(value);
    }

    void addNullToArray()
    {
        if (!arrayData) {
            throw JsonException("");
        }

        arrayData->emplace_back();
    }

    // Метод возвращает true, если данный экземпляр содержит в себе JSON-массив. Иначе false.
    bool is_array() const;
    // Метод возвращает true, если данный экземпляр содержит в себе JSON-объект. Иначе false.
    bool is_object() const;

    // TODO написать описание
    bool is_null() const;

    size_t getSize() const
    {
        if (arrayData) {
            return arrayData->size();
        } else if (objectData) {
            return objectData->size();
        } else {
            return 0;
        }
    }


    // Метод возвращает значение по ключу key, если экземпляр является JSON-объектом.
    // Значение может иметь один из следующих типов: Json, std::string, double, bool или быть пустым.
    // Если экземпляр является JSON-массивом, генерируется исключение.
    std::any &operator[](const std::string &key);

    // Метод возвращает значение по индексу index, если экземпляр является JSON-массивом.
    // Значение может иметь один из следующих типов: Json, std::string, double, bool или быть пустым.
    // Если экземпляр является JSON-объектом, генерируется исключение.
    std::any &operator[](int index);

    // Метод возвращает объект класса Json из строки, содержащей Json-данные.
    static Json parse(const std::string &s);

    // Метод возвращает объекта класса Json из файла, содержащего Json-данные в текстовом формате.
    static Json parseFile(const std::string &path_to_file);

    virtual ~Json();

private:
    ObjectType *objectData = nullptr;
    ArrayType *arrayData = nullptr;
};