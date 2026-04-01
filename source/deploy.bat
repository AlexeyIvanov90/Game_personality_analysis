@echo off
echo ========================================
echo Развертывание Qt приложения с QML
echo ========================================
echo.

REM Указываем правильный путь к windeployqt (MSVC2017)
set "WINDEPLOYQT=C:\Qt\5.14.2\msvc2017_64\bin\windeployqt.exe"
set "TARGET_EXE=%~dp0Game_personality_analysis.exe"

REM Проверяем существование файлов
if not exist "%WINDEPLOYQT%" (
    echo ОШИБКА: windeployqt.exe не найден по пути: %WINDEPLOYQT%
    pause
    exit /b 1
)

if not exist "%TARGET_EXE%" (
    echo ОШИБКА: %TARGET_EXE% не найден!
    pause
    exit /b 1
)

echo Развертывание для: %TARGET_EXE%
echo.

REM Ключевая команда: --quick --qmldir (указываем путь к qml из Qt)
"%WINDEPLOYQT%" "%TARGET_EXE%" --quick --qmldir "C:\Qt\5.14.2\msvc2017_64\qml"

echo.
echo ========================================
echo Развертывание завершено!
echo Теперь программа должна запускаться.
echo ========================================
pause