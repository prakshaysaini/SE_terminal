echo "Cleaning Build Artifacts..."

if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
else
    echo "Build directory not found"
fi

if ls *.db 1> /dev/null 2>&1; then
    echo "Removing test database files..."
    rm -f *.db
fi

if [ -f "test_report.txt" ]; then
    echo "Removing test_report.txt..."
    rm -f test_report.txt
fi

if [ -f "CMakeCache.txt" ]; then
    echo "Removing CMakeCache.txt..."
    rm -f CMakeCache.txt
fi

if [ -f "cmake_install.cmake" ]; then
    echo "Removing cmake_install.cmake..."
    rm -f cmake_install.cmake
fi

if [ -d "CMakeFiles" ]; then
    echo "Removing CMakeFiles directory..."
    rm -rf CMakeFiles
fi

echo ""
echo "Clean completed successfully!"
