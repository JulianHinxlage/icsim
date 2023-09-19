call build.bat

cd ..
rmdir /s /q deploy
mkdir deploy
cd build
cd Release

xcopy *.exe ..\..\deploy
xcopy *.dll ..\..\deploy

cd ..
cd ..
cd scripts
