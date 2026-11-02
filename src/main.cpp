#include "db_connector.hpp"
#include "db_writer.hpp"
#include "config_reader.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <csignal>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif

// Флаг для корректного завершения по Ctrl+C (SIGINT)
volatile sig_atomic_t g_running = 1;

void signalHandler(int /*signum*/) {
    g_running = 0;
}

// Значение для строки подключения libpq: в одинарных кавычках, ' и обратный слэш экранируются
// (иначе пароль с пробелом ломает разбор строки)
static std::string quoteConnValue(const std::string& v) {
    std::string r = "'";
    for (char c : v) {
        if (c == '\'' || c == '\\') r += '\\';
        r += c;
    }
    return r + "'";
}

int main() {
#ifdef _WIN32
    // Вывод в консоль Windows в UTF-8 (иначе кириллица отображается некорректно)
    SetConsoleOutputCP(CP_UTF8);
#endif
    // Регистрируем обработчик Ctrl+C
    std::signal(SIGINT, signalHandler);

    // Инициализируем ГСЧ один раз
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Читаем конфигурационный файл из текущей директории
    const char* configPath = "./func_sig_db_v2.cfg";
    AppConfig cfg = readConfig(configPath);
    std::cout << "Конфигурация загружена из " << configPath << "\n";

    // Пароль из конфига; если не задан — из переменной окружения PGPASSWORD
    std::string password = cfg.password;
    if (password.empty()) {
        const char* pwd = std::getenv("PGPASSWORD");
        if (pwd) password = pwd;
    }
    std::string conninfo = "dbname=" + quoteConnValue(cfg.dbname) +
                           (cfg.host.empty() ? "" : " host=" + quoteConnValue(cfg.host)) +
                           (cfg.port.empty() ? "" : " port=" + quoteConnValue(cfg.port)) +
                           " user=" + quoteConnValue(cfg.user) +
                           " password=" + quoteConnValue(password);
    try {
        DBConnector db(conninfo);
        DBWriter writer(cfg.tbl_name);
        std::cout << "Подключение к БД успешно.\n";
        std::cout << "Запись в режиме '" << cfg.func_mode<< "' случайных значений раз в " << cfg.cycle_time
                  << " мсек... (Ctrl+C для выхода)\n";

        // Цикл: обновляем Value случайными значениями с периодом из конфига
        while (g_running) {
            if (cfg.func_mode == "random") {
              writer.populateRandomValues(db);
            } else if (cfg.func_mode == "sine") {
              writer.populateSineValues(db, cfg.cycle_time, cfg.SinePeriodMS);
            } else if (cfg.func_mode == "start") {
              writer.populateRampValues(db, true);
            } else if (cfg.func_mode == "stop") {
              writer.populateRampValues(db, false);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(cfg.cycle_time));
        }

        std::cout << "\nЗавершение работы.\n";

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}