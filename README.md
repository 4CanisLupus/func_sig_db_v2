# func_sig_db_v2

Проект циклически подключается к PostgreSQL и записывает значения в таблицу сигналов согласно выбранному режиму генерации.

## Структура проекта

```
func_sig_db_v2/
├── CMakeLists.txt
├── README.md
├── func_sig_db_v2.cfg              # конфигурационный файл (в месте запуска)
├── config/
│   └── func_sig_db_v2.cfg          # резервная копия конфига
├── include/
│   ├── config_reader.hpp           # структура AppConfig + readConfig()
│   ├── db_connector.hpp            # класс DBConnector (подключение к БД)
│   └── db_writer.hpp               # класс DBWriter (генерация и запись значений)
└── src/
    ├── main.cpp                    # точка входа, главный цикл
    ├── config_reader.cpp           # парсер JSON-конфига
    ├── db_connector.cpp            # реализация DBConnector (libpq)
    └── db_writer.cpp               # реализация DBWriter
```

## Конфигурация

Программа читает настройки из JSON-файла `./func_sig_db_v2.cfg` (рядом с исполняемым файлом).

| Поле | Тип | Назначение |
|---|---|---|
| `dbname` | string | Имя базы данных |
| `user` | string | Пользователь БД |
| `tbl_name` | string | Имя целевой таблицы с схемой (`simul.tbl_cfa_simul_sig_0`) |
| `cycle_time` | string (int) | Интервал между циклами записи, **мс** |
| `func_mode` | string | Режим генерации: `"random"` или `"sine"` |
| `SinePeriodMS` | string (int) | Период полного оборота синусоиды, **мс** (только для `sine`) |

Пример:
```json
{
    "dbname": "purneft_P174",
    "user": "postgres",
    "tbl_name": "simul.tbl_cfa_simul_sig_0",
    "cycle_time": "1000",
    "func_mode": "sine",
    "SinePeriodMS": "60000"
}
```

## Режимы работы (`func_mode`)

### `random` — случайные значения
Генерирует случайные значения в зависимости от типа (`type`) каждой строки:
- `int` / `integer` / `uint` → случайное число [10000, 50000]
- `bool` / `boolean` → 0 или 1

### `sine` — синусоида ±10%
Считывает базовое значение из столбца `simul` и изменяет его по синусоиде:
`Value = simul × (1.0 + 0.1 × sin(phase))`

Фаза наращивается с каждым циклом пропорционально `cycle_time`, полный период задаётся полем `SinePeriodMS`.

## Управление записью через `sensor_cndtn`

Каждая строка таблицы должна иметь столбец `sensor_cndtn` (INTEGER), определяющий поведение записи:

| `sensor_cndtn` | Поведение |
|---|---|
| `0` | Строка пропускается — `Value` **не обновляется** |
| `1` | Штатная работа согласно `func_mode` |
| `2` | В `Value` записывается **0** |

## Требования к таблице БД

Таблица должна содержать следующие столбцы:

| Столбец | Тип | Назначение |
|---|---|---|
| `id` | SERIAL / INTEGER | Первичный ключ |
| `type` | TEXT | Тип сигнала (`int`, `bool`, ...) — для `random` |
| `simul` | TEXT / NUMERIC | Базовое значение — для `sine` |
| `"Value"` | TEXT / NUMERIC | Целевой столбец для записи |
| `sensor_cndtn` | INTEGER | Условие записи: 0=пропуск, 1=штатно, 2=ноль |

Пример создания таблицы:
```sql
CREATE SCHEMA IF NOT EXISTS simul;
CREATE TABLE simul.tbl_cfa_simul_sig_0 (
    id SERIAL PRIMARY KEY,
    type TEXT,
    simul TEXT,
    "Value" TEXT,
    sensor_cndtn INTEGER DEFAULT 1
);
```

## Сборка и запуск (Windows)

**Требования:** CMake ≥ 3.10, MinGW-w64 (MSYS2, `C:\msys64\mingw64`), libpq из MSYS2:
`pacman -S mingw-w64-x86_64-postgresql`. libpq из установщика PostgreSQL для Windows (MSVC) с MinGW не линкуется,
поэтому CMake берёт её из `PG_ROOT` (по умолчанию `C:/msys64/mingw64`).

```bat
build.bat
set PGPASSWORD=ваш_пароль
run.bat
```

Конфиг `func_sig_db_v2.cfg` копируется в `build\` при сборке; программа читает его из текущей директории (`run.bat` запускает из `build`).
Остановка — **Ctrl+C**.

## Переменные окружения

| Переменная | Назначение |
|---|---|
| `PGPASSWORD` | Пароль пользователя БД |
