#/bin/sh
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADER=0 -DFORBOOTLOADER=0 ..
make -j
cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADER=0 -DFORBOOTLOADER=1 ..
make -j
cmake -DCMAKE_BUILD_TYPE=Release -DBOOTLOADER=1 -DFORBOOTLOADER=0 ..
make -j
