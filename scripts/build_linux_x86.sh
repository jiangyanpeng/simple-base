#!/usr/bin/bash
rm -rf build && mkdir build && cd build

# cmake -DCMAKE_BUILD_TYPE=DEBUG -DBUILD_USE_STB=ON -DBUILD_USE_AVX=ON ..
cmake ..
# make -j4 && make install
make -j4
./bin/test_base