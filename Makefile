.PHONY: all

all: compile_client compile_server

compile_client: client_executable

client_executable: ./Client/client.c
	gcc -o ./Client/client ./Client/client.c -L./Client/ -lclient -lserver

compile_server: ./Server/server.c
	gcc -o ./Server/server ./Server/server.c -L./Server/ -lclient -lserver
