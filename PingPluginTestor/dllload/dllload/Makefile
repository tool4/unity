LIB_DIR=../../../PingPlugin/PluginSource/Linux/build/bin

all:
	g++ -g -Wall -Werror -O2 -fPIC -o ping_plugin_ult dllload.cpp -L $(LIB_DIR) -l:PingPlugin.so -pthread -Wl,-rpath=$(LIB_DIR)

clean:
	rm -f ping_plugin_ult
