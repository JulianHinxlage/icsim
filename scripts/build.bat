call cmake.bat

cd ..
mkdir build
cd build

msbuild icsim.sln /p:Configuration=Release

cd ..
cd scripts
