#include "config_reader.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>

namespace {

// Убирает пробельные символы по краям
std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

// Извлекает строковое значение из JSON-строки в кавычках (со входной позиции после открывающей кавычки)
std::string extractJsonString(const std::string& json, size_t& pos) {
    // pos указывает на первый символ после открывающей кавычки
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            // Простейшая обработка escape-последовательностей
            char next = json[pos + 1];
            if (next == '"')  { result += '"';  pos += 2; continue; }
            if (next == '\\') { result += '\\'; pos += 2; continue; }
            if (next == 'n')  { result += '\n'; pos += 2; continue; }
            if (next == 't')  { result += '\t'; pos += 2; continue; }
        }
        result += json[pos];
        ++pos;
    }
    if (pos < json.size() && json[pos] == '"') {
        ++pos; // пропускаем закрывающую кавычку
    }
    return result;
}

// Извлекает значение (строку или число) после двоеточия
std::string extractJsonValue(const std::string& json, size_t& pos) {
    // Пропускаем пробелы
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }

    if (pos >= json.size()) {
        return "";
    }

    if (json[pos] == '"') {
        ++pos; // пропускаем открывающую кавычку
        return extractJsonString(json, pos);
    }

    // Числовое или другое значение — читаем до запятой, } или пробела
    std::string val;
    while (pos < json.size() && json[pos] != ',' && json[pos] != '}' 
           && !std::isspace(static_cast<unsigned char>(json[pos]))) {
        val += json[pos];
        ++pos;
    }
    return val;
}

} // анонимный namespace

AppConfig readConfig(const std::string& filePath) {
    // Читаем весь файл в строку
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл конфигурации: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Простейший парсер JSON-объекта вида {"key": "value", ...}
    // Находим открывающую фигурную скобку
    size_t pos = content.find('{');
    if (pos == std::string::npos) {
        throw std::runtime_error("Некорректный формат JSON: не найден '{'");
    }
    ++pos; // переходим внутрь объекта

    AppConfig cfg;
    cfg.cycle_time = 1000;    // значение по умолчанию
    cfg.SinePeriodMS = 60000; // значение по умолчанию (60 сек)

    while (pos < content.size()) {
        // Пропускаем пробелы
        while (pos < content.size() && std::isspace(static_cast<unsigned char>(content[pos]))) {
            ++pos;
        }
        if (pos >= content.size() || content[pos] == '}') {
            break;
        }

        // Читаем ключ (строка в кавычках)
        if (content[pos] == '"') {
            ++pos;
            std::string key = extractJsonString(content, pos);
            key = trim(key);

            // Ищем двоеточие
            while (pos < content.size() && content[pos] != ':') {
                ++pos;
            }
            if (pos < content.size()) {
                ++pos; // пропускаем ':'
            }

            // Читаем значение
            std::string value = extractJsonValue(content, pos);
            value = trim(value);

            // Заполняем поля конфига
            if (key == "dbname") {
                cfg.dbname = value;
            } else if (key == "user") {
                cfg.user = value;
            } else if (key == "host") {
                cfg.host = value;
            } else if (key == "port") {
                cfg.port = value;
            } else if (key == "tbl_name") {
                cfg.tbl_name = value;
            } else if (key == "cycle_time") {
                cfg.cycle_time = std::atoi(value.c_str());
                if (cfg.cycle_time <= 0) {
                    cfg.cycle_time = 1000;
                }
            } else if (key == "func_mode") {
                cfg.func_mode = value;
            } else if (key == "SinePeriodMS") {
                cfg.SinePeriodMS = std::atoi(value.c_str());
                if (cfg.SinePeriodMS <= 0) {
                    cfg.SinePeriodMS = 60000;
                }
            }
        }

        // Пропускаем запятую, если есть
        while (pos < content.size() && content[pos] != ',' && content[pos] != '}') {
            ++pos;
        }
        if (pos < content.size() && content[pos] == ',') {
            ++pos;
        }
    }

    return cfg;
}
