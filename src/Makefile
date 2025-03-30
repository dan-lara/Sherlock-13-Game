# Compiler and flags
CC = gcc
CFLAGS = -I/usr/include/SDL2
LDFLAGS = -lSDL2_image -lSDL2_ttf -lSDL2 -lpthread

# Targets
all: check_dir_build server client 

client:
	$(CC) $(CFLAGS) -o build/client client.c $(LDFLAGS)
	@echo "Client built successfully."

server:
	$(CC) -o build/server server_v1.c 
	@echo "Server built successfully."


run_all : all
	@echo "Running server and client..."
	@cd build && ./server &
	@cd build && ./client
	@echo "Server and client are running."

run_client : all
	@echo "Running client..."
	@cd build && ./client
	@echo "Client is running."

run_server : all
	@echo "Running server..."
	@cd build && ./server
	@echo "Server is running."

help:
	@echo "Usage:"
	@echo "  make all          - Build both server and client"
	@echo "  make run_all      - Run both server and client"
	@echo "  make run_client   - Run only the client"
	@echo "  make run_server   - Run only the server"
	@echo "  make clean        - Clean up build files"
	@echo "  make help         - Show this help message"

clean:
	rm -rf build

check_dir_build:
ifeq ($(wildcard build),)
	$(shell mkdir build)
endif