#!/bin/bash

pushd .
PLUGIN_DIR=../../../PingPlugin/PluginSource/Linux/
cd $PLUGIN_DIR
./build.sh
popd

make clean all
sudo ./ping_plugin_ult
sudo ./ping_plugin_ult sync
