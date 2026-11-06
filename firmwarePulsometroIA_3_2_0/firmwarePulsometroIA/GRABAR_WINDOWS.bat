@echo off
setlocal
cd /d "%~dp0"
if "%~1"=="" (
  echo Uso: GRABAR_WINDOWS.bat COM5
  echo Reemplazar COM5 por el puerto de la placa.
  exit /b 1
)
if not exist "release\PULSOMETRO_ESP32_38P.bin" (
  echo No hay binario unificado. Abrir el proyecto en PlatformIO y ejecutar Build.
  exit /b 1
)
set "PULSO_PYTHON=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "PULSO_ESPTOOL=%USERPROFILE%\.platformio\packages\tool-esptoolpy\esptool.py"
if not exist "%PULSO_PYTHON%" (
  echo No se encontro Python de PlatformIO. Usar PlatformIO Upload.
  exit /b 1
)
if not exist "%PULSO_ESPTOOL%" (
  echo No se encontro esptool. Usar PlatformIO Upload.
  exit /b 1
)
"%PULSO_PYTHON%" "%PULSO_ESPTOOL%" --chip esp32 --port "%~1" --baud 115200 write_flash 0x0 "release\PULSOMETRO_ESP32_38P.bin"
exit /b %errorlevel%
