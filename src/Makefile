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

# Default port for the server
SPort ?= 8000
SIP ?= 127.0.0.1
CIP ?= 127.0.0.1
CPort ?= 8001

run_client : all
	@echo "Running client with CPort=$(word 2, $(MAKECMDGOALS)) and PlayerName=$(word 3, $(MAKECMDGOALS))..."
	@cd build && ./client $(SIP) $(SPort) $(CIP) $(word 2, $(MAKECMDGOALS)) $(word 3, $(MAKECMDGOALS))
	@echo "Client is running."

run_server : all
	@echo "Running server on IP $(SIP) and port $(wordlist 2, 2, $(MAKECMDGOALS))..."
	@cd build && ./server $(SIP) $(wordlist 2, 2, $(MAKECMDGOALS))
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