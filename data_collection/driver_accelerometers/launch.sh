#!/bin/bash
set -e # stop on error

SLIP_PORT=4000

# Print help text and exit
function helptext {
  echo "$0 data/base/path /dev/tty.asdf"
  echo "Script to launch the accelerometer data logging slip client. We also launch the slipstream server on the same port and wait for it to initialize."
  exit -1
}

# process args
if [[ "$1" == "" ]] || [[ "$2" == "" ]]; then
  echo "Missing required args!"
  helptext
fi
data_path=$1
serial_device_path=$2

# change to script directory
cd `dirname $0`

# launch slipstream server
./slipstream_server "$serial_device_path" "$SLIP_PORT" &
sleep 1

# launch accelerometer data logging
./driver_accelerometers "$data_path" localhost "$SLIP_PORT"
