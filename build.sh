#!/bin/bash
pushd build
cmake .. -DDebug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
cmake --build .
popd
