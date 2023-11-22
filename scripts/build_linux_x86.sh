#!/usr/bin/bash
rm -rf build && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=simple.base/ -DBUILD_USE_AVX=ON -DBUILD_TEST=ON ..
make -j4 && make install
# ./simple.base/bin/test_base
