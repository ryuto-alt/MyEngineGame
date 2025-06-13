@echo off
echo Building Stage System...
cd /d "C:\Users\ryuto\OneDrive\ドキュメント\GitHub\MyEngineGame"

echo Compiling Stage System files...
cl /c /EHsc /std:c++17 /I"src" /I"src/Engine" src/Engine/Stage/StageGrid.cpp src/Engine/Stage/StagePart.cpp src/Engine/Stage/StageBuilder.cpp src/Engine/Stage/StageSerializer.cpp

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo Build completed successfully!
pause