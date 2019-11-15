#/bin/sh
mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADED=1 ..
make -j
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADER=1 ..
make -j
