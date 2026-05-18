# cpp_linter

`cpp_linter` — консольный статический анализатор кода для учебного подмножества C++. Программа рекурсивно проверяет C++-проект, находит файлы `.cpp`, `.h`, `.hpp`, выполняет лексический анализ, применяет правила стиля и простые проверки типовых ошибок, а затем выводит понятный отчет с предупреждениями, ошибками, рекомендациями и итоговой сводкой.

## Возможности

- Рекурсивный обход директории проекта.
- Фильтрация файлов `.cpp`, `.h`, `.hpp`.
- Исключение директорий и отдельных файлов.
- Чтение файлов и лексический анализ.
- Режимы проверки:
  - `style` — только стилевые правила;
  - `full` — стилевые правила и проверки типовых ошибок.
- Настройка правил через INI-конфигурацию.
- Отключение отдельных правил через CLI.
- Уровни серьезности: предупреждение и ошибка.
- Понятный отчет с файлом, строкой, колонкой, правилом, сообщением и рекомендацией.
- Итоговая сводка по количеству файлов, предупреждений и ошибок.
- Интерактивный режим.
- Сохранение конфигурации.
- Вывод прогресса проверки.
- Unit-тесты и тестовые сценарии через CTest.

## Правила

Стилевые правила:

- `STYLE-INDENTATION` — отступы, табуляция и смешанные отступы;
- `STYLE-SPACING` — пробелы вокруг бинарных операторов и после запятых;
- `STYLE-LINE-LENGTH` — максимальная длина строки;
- `STYLE-NAMING` — именование переменных, функций и констант.

Проверки типовых ошибок:

- `BUG-USE-BEFORE-INIT` — использование локальной переменной до инициализации;
- `BUG-MEMORY-LEAK` — простые случаи `new` без `delete`, `new[]` без `delete[]`, подозрительные пары освобождения памяти.

## Требования

- Windows.
- PowerShell.
- CMake 3.16 или новее.
- Компилятор C++17, например MSVC.
- Catch2 v3 для тестов. Если Catch2 не найден, CMake загрузит его автоматически через `FetchContent`.

## Сборка

Настроить проект:

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
```

Собрать проект:

```powershell
cmake --build build --config Release
```

Запустить тесты:

```powershell
ctest --test-dir build --output-on-failure -C Release
```

После сборки исполняемый файл доступен по пути:

```powershell
.\build\cpp_linter.exe
```

## Запуск

Показать справку:

```powershell
.\build\cpp_linter.exe --help
```

Проверить демонстрационный проект:

```powershell
.\build\cpp_linter.exe --project .\sample_project
```

Запустить проверку с конфигурацией:

```powershell
.\build\cpp_linter.exe --project .\sample_project --config .\config.example.ini
```

Запустить интерактивный режим:

```powershell
.\build\cpp_linter.exe --interactive
```

## Режимы проверки

Запустить только стилевые проверки:

```powershell
.\build\cpp_linter.exe --project .\sample_project --mode style
```

Запустить полную проверку:

```powershell
.\build\cpp_linter.exe --project .\sample_project --mode full
```

В режиме `style` включаются:

- `STYLE-INDENTATION`
- `STYLE-SPACING`
- `STYLE-LINE-LENGTH`
- `STYLE-NAMING`

В режиме `full` дополнительно включаются:

- `BUG-USE-BEFORE-INIT`
- `BUG-MEMORY-LEAK`

## Отключение правил

Отключить проверку пробелов:

```powershell
.\build\cpp_linter.exe --project .\sample_project --disable STYLE-SPACING
```

Параметр `--disable` применяется поверх режима и поверх конфигурационного файла.

## Исключение файлов и директорий

Исключить директорию:

```powershell
.\build\cpp_linter.exe --project .\sample_project --exclude-dir build
```

Исключить файл:

```powershell
.\build\cpp_linter.exe --project .\sample_project --exclude-file style_problems.cpp
```

По умолчанию исключаются:

- `build`
- `.git`
- `third_party`
- `cmake-build-debug`
- `cmake-build-release`

## Прогресс анализа

Показать ход проверки:

```powershell
.\build\cpp_linter.exe --project .\sample_project --mode full --show-progress
```

В этом режиме программа выводит, какой файл анализируется, сколько файлов уже обработано, и итоговую краткую сводку.

## Конфигурация

Пример конфигурации находится в файле `config.example.ini`.

Основные секции:

- `[general]` — режим проверки и вывод прогресса;
- `[style]` — параметры форматирования и именования;
- `[rules]` — включенные и отключенные правила;
- `[severity]` — уровни серьезности;
- `[exclude]` — исключаемые директории и файлы.

Сохранить итоговую конфигурацию:

```powershell
.\build\cpp_linter.exe --project .\sample_project --config .\config.example.ini --save-config .\saved.ini
```

Если в `--save-config` указать существующую папку, конфигурация будет сохранена в файл `config.ini` внутри этой папки.

## Формат отчета

Для каждой найденной проблемы выводится строка:

```text
file:line:column: уровень: rule-id: сообщение
рекомендация: ...
```

В конце отчета выводится сводка:

```text
проверено файлов: N
предупреждений: N
ошибок: N
всего нарушений: N
```

Если нарушений нет, программа пишет:

```text
Нарушений не найдено.
```

## Тесты

Все тесты запускаются через CTest:

```powershell
ctest --test-dir build --output-on-failure -C Release
```

В тестах есть:

- unit-тесты для конфигурации, сканера файлов, токенизатора, фабрики правил, правил анализа, движка, отчета и `ConfigManager`;
- сценарии для анализа sample-проекта, сохранения и загрузки конфигурации, observer flow, интерактивной настройки и большого проекта.

## Структура проекта

```text
pks-linter-cpp/
├── CMakeLists.txt
├── README.md
├── config.example.ini
├── include/
├── src/
├── sample_project/
└── tests/
    ├── CMakeLists.txt
    ├── unit/
    └── scenarios/
```

## Паттерны проектирования

- «Стратегия»: каждое правило реализует общий интерфейс `Rule`.
- «Фабричный метод»: `RuleFactory` создает правила по идентификаторам и конфигурации.
- «Наблюдатель»: `AnalyzerEngine` уведомляет observers о начале файла, найденных проблемах, конце файла и завершении анализа.
- «Одиночка»: `ConfigManager` хранит единую конфигурацию на время запуска.

## Сторонние библиотеки

- Catch2 v3 — используется для тестов.
