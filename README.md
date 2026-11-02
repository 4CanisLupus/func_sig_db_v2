# func_sig_db_v2

Проект циклически подключается к PostgreSQL и записывает значения в таблицу сигналов согласно выбранному режиму генерации.

## Структура проекта

```
func_sig_db_v2/
├── CMakeLists.txt
├── README.md
├── build.bat                       # сборка под Windows (MinGW)
├── run.bat                         # запуск из папки build
├── func_sig_db_v2.cfg              # конфигурационный файл (копируется в build\ при сборке)
├── config/
│   └── func_sig_db_v2.cfg          # резервная копия конфига
├── .vscode/                        # tasks.json, launch.json, c_cpp_properties.json (MinGW/gdb)
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

Программа читает настройки из JSON-файла `./func_sig_db_v2.cfg` (из текущей директории; при запуске через `run.bat` или отладку VS Code — это `build\`).

| Поле | Тип | Назначение |
|---|---|---|
| `dbname` | string | Имя базы данных |
| `user` | string | Пользователь БД |
| `password` | string | Пароль пользователя БД (необязательно; если пусто — берётся из переменной окружения `PGPASSWORD`) |
| `host` | string | Хост БД (необязательно, по умолчанию `localhost`) |
| `port` | string | Порт БД (необязательно, по умолчанию `5432`; для текущего сервера — `5433`) |
| `tbl_name` | string | Имя целевой таблицы со схемой (`simul.tbl_cfa_simul_sig_0`) |
| `cycle_time` | string (int) | Интервал между циклами записи, **мс** |
| `func_mode` | string | Режим генерации: `"random"` или `"sine"` |
| `SinePeriodMS` | string (int) | Период полного оборота синусоиды, **мс** (только для `sine`) |

Пример:
```json
{
    "dbname": "purneft_P174",
    "user": "postgres",
    "password": "ваш_пароль",
    "host": "localhost",
    "port": "5433",
    "tbl_name": "simul.tbl_cfa_simul_sig_0",
    "cycle_time": "1000",
    "func_mode": "sine",
    "SinePeriodMS": "60000"
}
```

> Файлы `*.cfg` добавлены в `.gitignore` и не попадают в репозиторий, так как содержат пароль. После клонирования создайте `func_sig_db_v2.cfg` вручную по примеру выше.

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
run.bat
```

Пароль задаётся в `func_sig_db_v2.cfg` (поле `password`); при необходимости его можно передать через `set PGPASSWORD=ваш_пароль` перед `run.bat`.

Конфиг `func_sig_db_v2.cfg` копируется в `build\` при сборке; программа читает его из текущей директории (`run.bat` запускает из `build`).
Остановка — **Ctrl+C**.

### Отладка в VS Code
Нужны расширение C/C++ и gdb (`pacman -S mingw-w64-x86_64-gdb`). Конфигурация **Debug func_sig_db** собирает проект задачей `Build` и запускает `build/func_sig_db.exe`. Пароль берётся из `func_sig_db_v2.cfg` (при пустом `password` — из `PGPASSWORD` в `.vscode/launch.json`, этот файл в `.gitignore`).

### Типичные проблемы
- **`no password supplied` / `password authentication failed`** — не задан или неверен `password` в конфиге (или `PGPASSWORD`); проверьте вход: `psql -U postgres -h localhost -p 5433 -d purneft_P174`.
- **Подключение не к тому серверу** — на машине может быть несколько экземпляров PostgreSQL; фактический порт узнайте запросом `SHOW port;` и укажите его в `port`.
- **Нечитаемый текст ошибки подключения** — сервер присылает сообщение об аутентификации в кодировке Windows-1251; смысл см. по контексту (`password authentication failed`).

## Переменные окружения

| Переменная | Назначение |
|---|---|
| `PGPASSWORD` | Пароль пользователя БД (используется, если `password` в конфиге пуст) |
