#ifndef CONFIG_READER_HPP
#define CONFIG_READER_HPP

#include <string>
#include <stdexcept>

struct AppConfig {
    std::string dbname;
    std::string user;
    std::string host; // необязательно, по умолчанию localhost
    std::string port; // необязательно, порт PostgreSQL (по умолчанию 5432)
    std::string tbl_name;
    int cycle_time;   // миллисекунды, интервал между вызовами
    std::string func_mode;
    int SinePeriodMS; // миллисекунды, период полного оборота синусоиды
};

// Читает JSON-конфиг из файла. Выбрасывает std::runtime_error при ошибке.
AppConfig readConfig(const std::string& filePath);

#endif // CONFIG_READER_HPP
