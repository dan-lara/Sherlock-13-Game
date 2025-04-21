#! /bin/sh

# Create the build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Change to the build directory
cd build

# Compile the application
gcc -o client -I/usr/include/SDL2 ../sh13_v1.c -lSDL2_image -lSDL2_ttf -lSDL2 -lpthread
gcc -o server ../server_v1.c -lpthread

# Check if the compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable files created: sh13 and server"
else
    echo "Compilation failed."
fi
# Change back to the original directory
cd ..