#ifndef DB_WRITER_HPP
#define DB_WRITER_HPP

#include "db_connector.hpp"
#include <string>
#include <cmath>
#include <chrono>

class DBWriter {
public:
    // Конструктор принимает имя таблицы, с которой будет работать writer
    explicit DBWriter(const std::string& tableName);

    // Заполняет колонку Value случайными значениями в зависимости от типа (Type)
    // int  -> значения от 10000 до 50000
    // bool -> 0 или 1
    void populateRandomValues(DBConnector& db);

    // Считывает simul, изменяет на ±10% по синусоиде и записывает в Value
    // cycleTimeMs  — интервал между вызовами (из конфига), задаёт шаг фазы
    // sinePeriodMs — период полного оборота синусоиды в мс (из конфига)
    void populateSineValues(DBConnector& db, int cycleTimeMs, int sinePeriodMs);

    // Режимы "start"/"stop": линейное изменение Value для строк с sensor_alg = 'line'
    // start: sim_min -> sim_max за start_time мс; stop: sim_max -> sim_min за stop_time мс
    // Время отсчитывается от первого вызова; после окончания значение удерживается на конце.
    // Меняются только строки с sensor_cndtn = 1.
    void populateRampValues(DBConnector& db, bool start);

private:
    std::string m_tableName;
    bool m_rampStarted = false;
    std::chrono::steady_clock::time_point m_rampT0;
    double m_sinePhase = 0.0; // фаза синусоиды, накапливается с каждым вызовом
};

#endif // DB_WRITER_HPP