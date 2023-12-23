.PHONY: all

all: compile_client compile_server add_lib

compile_client: client_executable

add_lib:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./Client/:./Server/


client_executable: ./Client/client.c
	gcc -o ./Client/client ./Client/client.c -L./Client/ -lclient -lserver -lssl -lcrypto -lpcre -O3

compile_server: ./Server/server.c
	gcc -o ./Server/server ./Server/server.c -L./Server/ -lclient -lserver -lssl -lcrypto -lpcre -O3
