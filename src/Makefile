# Compiler and flags
CC = gcc
CFLAGS = -I/usr/include/SDL2
LDFLAGS = -lSDL2_image -lSDL2_ttf -lSDL2 -lpthread

VERSION = v1
# Source files
ifeq ($(VERSION),v1)
SRC_SERVER = server_v1.c
SRC_CLIENT = sh13_v1.c
else ifeq ($(VERSION),v2)
SRC_SERVER = server.c
SRC_CLIENT = client.c
else
$(error "Invalid VERSION specified. Use VERSION=v1 or VERSION=v2.")
endif

# Targets
all: check_dir_build server client 

client:
	$(CC) $(CFLAGS) -o build/client $(SRC_CLIENT) $(LDFLAGS)
	@echo "Client built successfully."

server:
	$(CC) -o build/server $(SRC_SERVER)
	@echo "Server built successfully."

# Default port for the server
SPort ?= 30000
SIP ?= localhost
CIP ?= localhost
CPort ?= 40000


run_server : all
	@echo "Running server on IP $(SIP) and port $(wordlist 2, 2, $(MAKECMDGOALS))..."
	@cd build && ./server $(SIP) $(wordlist 2, 2, $(MAKECMDGOALS))
	@echo "Server is running."

run_client : all
	@echo "Running client on IP $(CIP) and port $(wordlist 2, 2, $(MAKECMDGOALS))... User: $(wordlist 3, 3, $(MAKECMDGOALS))"
	@cd build && ./client $(SIP) $(SPort) $(CIP) $(wordlist 2, 2, $(MAKECMDGOALS)) $(wordlist 3, 3, $(MAKECMDGOALS))
	@echo "Client is running."

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