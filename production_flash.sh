#/bin/sh
rm -rf build
mkdir -p build
cd build
cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADER=1 ..
ninja ; sudo ninja install
rm -rf *
cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADED=1 ..
ninja ; sudo ninja install
rm -rf *
