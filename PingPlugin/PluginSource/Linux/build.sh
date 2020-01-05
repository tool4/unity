#!/bin/bash
set -e
mkdir -p build build/bin build/obj

g++ -g -c -O2 -fPIC -o build/obj/Plugin.o -I.. ../Plugin.cpp
g++ -g -c -O2 -fPIC -o build/obj/Pinger.o -I.. ../Pinger.cpp
g++ -shared -o build/bin/PingPlugin.so build/obj/Plugin.o build/obj/Pinger.o
objcopy --only-keep-debug build/bin/PingPlugin.so build/bin/PingPlugin.debug
strip --strip-debug build/bin/PingPlugin.so
cp -t "../../Unity Project/Assets/Plugins/Linux/" build/bin/PingPlugin.so build/bin/PingPlugin.debug
