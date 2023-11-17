#!/usr/bin/bash
rm -rf build && mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=. -DBUILD_USE_AVX=ON ..

make -j4 && make install
./bin/test_base