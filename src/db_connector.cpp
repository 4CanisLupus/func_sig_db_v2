#include "db_connector.hpp"
#include <iostream>

DBConnector::DBConnector(const std::string& conninfo) : conn(nullptr) {
    conn = PQconnectdb(conninfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::string err = PQerrorMessage(conn);
        PQfinish(conn);
        throw std::runtime_error("Ошибка подключения к БД: " + err);
    }
    // Устанавливаем кодировку UTF-8 (опционально)
    PQsetClientEncoding(conn, "UTF8");
}

DBConnector::~DBConnector() {
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}

bool DBConnector::isConnected() const {
    return conn && PQstatus(conn) == CONNECTION_OK;
}

void DBConnector::checkConnection() {
    if (!isConnected()) {
        throw std::runtime_error("Соединение с БД потеряно");
    }
}

std::vector<std::vector<std::string>> DBConnector::executeQuery(const std::string& query) {
    checkConnection();

    PGresult* res = PQexec(conn, query.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::string err = PQerrorMessage(conn);
        PQclear(res);
        throw std::runtime_error("Ошибка выполнения запроса: " + err);
    }

    int rows = PQntuples(res);
    int cols = PQnfields(res);
    std::vector<std::vector<std::string>> result;

    for (int i = 0; i < rows; ++i) {
        std::vector<std::string> row;
        for (int j = 0; j < cols; ++j) {
            // NULL-значения заменяем на пустую строку (можно изменить)
            if (PQgetisnull(res, i, j)) {
                row.push_back("");
            } else {
                row.push_back(PQgetvalue(res, i, j));
            }
        }
        result.push_back(row);
    }

    PQclear(res);
    return result;
}

void DBConnector::executeCommand(const std::string& command) {
    checkConnection();

    PGresult* res = PQexec(conn, command.c_str());
    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        std::string err = PQerrorMessage(conn);
        PQclear(res);
        throw std::runtime_error("Ошибка выполнения команды: " + err);
    }
    PQclear(res);
}