# Команды для cpp_linter (Windows PowerShell)

## 1. Настроить сборку

```powershell
cmake -S . -B build
```

## 2. Собрать проект

```powershell
cmake --build build --config Release
```

## 3. Перейти в папку с исполняемым файлом

```powershell
cd build
```

## 4. Проверить, что программа запускается

```powershell
.\cpp_linter.exe --help
```

## 5. Запуск анализа проекта

```powershell
.\cpp_linter.exe --project ..\sample_project
```

## 6. Запуск с конфигурацией

```powershell
.\cpp_linter.exe --project ..\sample_project --config ..\config.example.ini
```

## 7. Интерактивный режим

```powershell
.\cpp_linter.exe --interactive
```

## 8. Исключение директорий из анализа

```powershell
.\cpp_linter.exe --project .. --exclude build --exclude .git --exclude third_party
```

## 9. Отключение конкретного правила

```powershell
.\cpp_linter.exe --project ..\sample_project --disable STYLE-SPACING
```

## 10. Сохранить конфигурацию из интерактивного режима

```powershell
.\cpp_linter.exe --interactive --save-config ..\my_config.ini
```

## 11. Запуск с абсолютными путями

```powershell
.\cpp_linter.exe --project "C:\вуз\пкс\задания\linter\sample_project" --config "C:\вуз\пкс\задания\linter\config.example.ini"
```

## 12. Очистить и пересобрать проект

```powershell
Set-Location C:\вуз\пкс\задания\linter
Remove-Item .\build -Recurse -Force
cmake -S . -B build
cmake --build build --config Release
```

## 13. Запуск unit-тестов

```powershell
.\build\tests\cpp_linter_unit_tests.exe
```

## 14. Запуск мини-сценариев

```powershell
.\build\tests\scenarios\scenario_config_roundtrip.exe
.\build\tests\scenarios\scenario_sample_analysis.exe
.\build\tests\scenarios\scenario_observer_flow.exe
```
