@echo off
REM Crest Build Script for Windows

echo Building Crest...

REM Clean previous builds
echo Cleaning previous builds...
if exist build rmdir /s /q build
if exist .xmake rmdir /s /q .xmake

REM Configure
echo Configuring build...
xmake config -m release

REM Build library
echo Building library...
xmake build crest

REM Build examples
echo Building examples...
xmake build crest_example_cpp
xmake build crest_example_c

REM Build tests
echo Building tests...
xmake build crest_tests

echo.
echo Build complete!
echo.
echo Run examples:
echo   xmake run crest_example_cpp
echo   xmake run crest_example_c
echo.
echo Run tests:
echo   xmake run crest_tests
