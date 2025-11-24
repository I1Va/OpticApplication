#! /bin/bash

PLUGIN_NAME=libIAGraphicsPlugin.so
PROJECT_NAME=OpticApplication2

rm ./external/"$PLUGIN_NAME"

cd ~/Work/Ded/AppsWorkspace/Plugins/IAGraphicsPlugin

cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DSANITIZE=ON
cmake --build build
cp ./build/"$PLUGIN_NAME" ~/Work/Ded/AppsWorkspace/"$PROJECT_NAME"/external

cd ~/Work/Ded/AppsWorkspace/"$PROJECT_NAME"
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DSANITIZE=ON
cmake --build build

./build/"$PROJECT_NAME" ./external/"$PLUGIN_NAME"

