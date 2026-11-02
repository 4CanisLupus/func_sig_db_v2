#include "db_writer.hpp"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <string>

namespace {
constexpr double kPi = 3.14159265358979323846; // M_PI не определён в MSVC без _USE_MATH_DEFINES
}

DBWriter::DBWriter(const std::string& tableName)
    : m_tableName(tableName) {}

void DBWriter::populateRandomValues(DBConnector& db) {
    // Получаем все строки таблицы: id, type, sensor_cndtn
    std::string selectQuery = "SELECT id, type, sensor_cndtn FROM " + m_tableName + ";";
    auto rows = db.executeQuery(selectQuery);

    if (rows.empty()) {
        std::cout << "Таблица " << m_tableName << " пуста – нечего заполнять.\n";
        return;
    }

    for (const auto& row : rows) {
        const std::string& id = row[0];
        std::string type = row[1];
        int sensorCndtn = std::atoi(row[2].c_str());

        // Проверка условия записи
        if (sensorCndtn == 0) {
            // Не писать — пропускаем
            continue;
        }

        std::string newValue;
        if (sensorCndtn == 2) {
            // Писать 0
            newValue = "0";
        } else {
            // sensorCndtn == 1 — штатный алгоритм
            // Приводим тип к нижнему регистру для надёжного сравнения
            std::transform(type.begin(), type.end(), type.begin(), ::tolower);

            if (type == "int" || type == "integer" || type == "uint") {
                int val = 10000 + (std::rand() % 40001); // [10000, 50000]
                newValue = std::to_string(val);
            } else if (type == "bool" || type == "boolean") {
                int val = std::rand() % 2;              // 0 или 1
                newValue = std::to_string(val);
            } else {
                std::cout << "Пропущена строка id=" << id
                          << " (неизвестный тип: " << type << ")\n";
                continue;
            }
        }

        std::string updateQuery =
            "UPDATE " + m_tableName + " SET \"Value\" = " + newValue +
            " WHERE id = " + id + ";";
        db.executeCommand(updateQuery);
    }

    std::cout << "Колонка Value заполнена рандомнымизначениями.\n";
}

void DBWriter::populateSineValues(DBConnector& db, int cycleTimeMs, int sinePeriodMs) {
    // Наращиваем фазу пропорционально прошедшему времени
    double fullPeriodMs = static_cast<double>(sinePeriodMs);
    m_sinePhase += 2.0 * kPi * static_cast<double>(cycleTimeMs) / fullPeriodMs;
    // Заворачиваем фазу в [0, 2π) чтобы избежать неограниченного роста
    if (m_sinePhase > 2.0 * kPi) {
        m_sinePhase -= 2.0 * kPi;
    }

    // Читаем id, базовое значение simul и условие sensor_cndtn из таблицы
    std::string selectQuery = "SELECT id, simul, sensor_cndtn FROM " + m_tableName + ";";
    auto rows = db.executeQuery(selectQuery);

    if (rows.empty()) {
        std::cout << "Таблица " << m_tableName << " пуста – нечего заполнять.\n";
        return;
    }

    for (const auto& row : rows) {
        const std::string& id = row[0];
        const std::string& simulStr = row[1];
        int sensorCndtn = std::atoi(row[2].c_str());

        // Проверка условия записи
        if (sensorCndtn == 0) {
            // Не писать — пропускаем
            continue;
        }

        double newValue;
        if (sensorCndtn == 2) {
            // Писать 0
            newValue = 0.0;
        } else {
            // sensorCndtn == 1 — штатный алгоритм по синусоиде
            if (simulStr.empty()) {
                std::cout << "Пропущена строка id=" << id
                          << " (пустой simul)\n";
                continue;
            }

            double simulVal;
            try {
                simulVal = std::stod(simulStr);
            } catch (const std::exception&) {
                std::cout << "Пропущена строка id=" << id
                          << " (нечисловой simul: " << simulStr << ")\n";
                continue;
            }

            // ±10% по синусоиде: factor ∈ [0.9, 1.1]
            double factor = 1.0 + 0.1 * std::sin(m_sinePhase);
            newValue = simulVal * factor;
        }

        std::string updateQuery =
            "UPDATE " + m_tableName + " SET \"Value\" = " +
            std::to_string(newValue) + " WHERE id = " + id + ";";
        db.executeCommand(updateQuery);
    }

    std::cout << "Колонка Value обновлена по синусоиде (фаза="
              << m_sinePhase << " рад).\n";
}