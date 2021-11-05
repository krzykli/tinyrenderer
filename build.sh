#!/bin/bash
pushd build
cmake .. -BDebug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
cmake --build .
popd
