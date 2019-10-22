#/bin/sh
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADER=0 -DBOOTLOADED=0 ..
make -j
cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADER=0 -DBOOTLOADED=1 ..
make -j
cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADER=1 -DBOOTLOADED=0 ..
make -j
