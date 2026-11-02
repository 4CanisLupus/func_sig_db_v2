#ifndef DB_CONNECTOR_H
#define DB_CONNECTOR_H

#include <libpq-fe.h>
#include <string>
#include <vector>
#include <stdexcept>

class DBConnector {
public:
    // Конструктор принимает параметры подключения (строка соединения)
    explicit DBConnector(const std::string& conninfo);
    ~DBConnector();

    // Выполнить произвольный SQL-запрос, вернуть результат в виде вектора строк (каждая строка – вектор полей)
    std::vector<std::vector<std::string>> executeQuery(const std::string& query);

    // Выполнить команду (INSERT, UPDATE, DELETE и т.п.), не возвращающую строк
    void executeCommand(const std::string& command);

    // Проверить, установлено ли соединение
    bool isConnected() const;

private:
    PGconn* conn;
    void checkConnection(); // выбрасывает исключение при ошибке подключения
};

#endif