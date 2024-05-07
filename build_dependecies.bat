@echo off
setlocal

set WORING_DIR=%~dp0

set DEPENDENCY_DIR=%WORING_DIR%dependency
set VENDOR_DIR=%WORING_DIR%DLengine\vendor

REM Delete old build files
rmdir "%DEPENDENCY_DIR%" /s /q

REM Build assimp
cd "%VENDOR_DIR%\assimp"
rmdir ".\build" /s /q
cmake -S . -B .\build -A x64 -DASSIMP_BUILD_TESTS=OFF -DASSIMP_NO_EXPORT=ON -DBUILD_SHARED_LIBS=ON
cmake --build .\build --config Debug
cmake --build .\build --config Release

robocopy ".\include\assimp" "%DEPENDENCY_DIR%\include\assimp" /e /mir /np /mt /z /xf *.in
xcopy ".\build\include\assimp" "%DEPENDENCY_DIR%\include\assimp" /e /i /s /y

robocopy ".\build\bin\Debug" "%DEPENDENCY_DIR%\bin\Debug" *.dll /np /mt /z
robocopy ".\build\bin\Release" "%DEPENDENCY_DIR%\bin\Release" *.dll /np /mt /z

robocopy ".\build\lib\Debug" "%DEPENDENCY_DIR%\lib\Debug" *.lib /np /mt /z
robocopy ".\build\lib\Release" "%DEPENDENCY_DIR%\lib\Release" *.lib /np /mt /z

REM Build spdlog
cd "%VENDOR_DIR%\spdlog"
rmdir ".\build" /s /q
cmake -S . -B .\build -A x64 -DSPDLOG_BUILD_EXAMPLE=OFF -DSPDLOG_BUILD_SHARED=ON
cmake --build .\build --config Release

robocopy ".\include\spdlog" "%DEPENDENCY_DIR%\include\spdlog" /e /mir /np /mt /z

robocopy ".\build\Release" "%DEPENDENCY_DIR%\bin\Debug" *.dll /np /mt /z
robocopy ".\build\Release" "%DEPENDENCY_DIR%\lib\Debug" *.lib /np /mt /z

endlocal