@echo off
echo ========================================
echo Crest Framework - Running All Tests
echo ========================================
echo.

echo Building all tests...
xmake build crest_tests crest_test_middleware crest_test_websocket crest_test_database crest_test_upload crest_test_template
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo ========================================
echo Running Tests
echo ========================================

echo.
echo [1/6] Basic Tests...
xmake run crest_tests
if %errorlevel% neq 0 (
    echo Basic tests failed!
    exit /b 1
)

echo.
echo [2/6] Middleware Tests...
xmake run crest_test_middleware
if %errorlevel% neq 0 (
    echo Middleware tests failed!
    exit /b 1
)

echo.
echo [3/6] WebSocket Tests...
xmake run crest_test_websocket
if %errorlevel% neq 0 (
    echo WebSocket tests failed!
    exit /b 1
)

echo.
echo [4/6] Database Tests...
xmake run crest_test_database
if %errorlevel% neq 0 (
    echo Database tests failed!
    exit /b 1
)

echo.
echo [5/6] File Upload Tests...
xmake run crest_test_upload
if %errorlevel% neq 0 (
    echo File upload tests failed!
    exit /b 1
)

echo.
echo [6/6] Template Engine Tests...
xmake run crest_test_template
if %errorlevel% neq 0 (
    echo Template tests failed!
    exit /b 1
)

echo.
echo ========================================
echo ✅ ALL TESTS PASSED!
echo ========================================
echo.
echo Test Summary:
echo   - Basic Tests: PASSED
echo   - Middleware Tests: PASSED
echo   - WebSocket Tests: PASSED
echo   - Database Tests: PASSED
echo   - File Upload Tests: PASSED
echo   - Template Engine Tests: PASSED
echo.
echo Total: 6/6 test suites passed
echo ========================================
