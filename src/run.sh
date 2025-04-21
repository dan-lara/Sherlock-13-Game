#!/bin/bash

# Define variables
SIP=localhost
SPORT=30000
CIP=localhost
CPORT_BASE=40000
NAMES=("client1" "client2" "client3" "client4")
# Check if the build directory exists
if [ ! -d "build" ]; then
    echo "Build directory does not exist. Please run cmd.sh first."
    exit 1
fi

# Change to the build directory
cd build

# Run the server in a new terminal
gnome-terminal --tab -- bash -c "echo 'Starting server...'; ./server $SPORT; exec bash"

# # Run clients in new terminal tabs without gdb
# gnome-terminal -- bash -c "echo 'Starting client1...'; ./sh13 $SIP $SPORT $CIP $((CPORT_BASE + 1)) ${NAMES[0]} || { echo 'Client1 failed'; exit 1; }; exec bash"
# gnome-terminal -- bash -c "echo 'Starting client2...'; ./sh13 $SIP $SPORT $CIP $((CPORT_BASE + 2)) ${NAMES[1]}; exec bash"
# gnome-terminal -- bash -c "echo 'Starting client3...'; ./sh13 $SIP $SPORT $CIP $((CPORT_BASE + 3)) ${NAMES[2]}; exec bash"
# gnome-terminal -- bash -c "echo 'Starting client4...'; ./sh13 $SIP $SPORT $CIP $((CPORT_BASE + 4)) ${NAMES[3]}; exec bash"
