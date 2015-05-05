#!/bin/bash
set -e # stop on error

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

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
serial_device_path=$1
data_path=$2

# figure out script path from how this script was called
calling_dir=$(dirname "$0")

# launch slipstream server
$calling_dir/slipstream_server "$serial_device_path" "$SLIP_PORT" &
sleep 1

# launch accelerometer data logging
$calling_dir/driver_accelerometers localhost "$SLIP_PORT" "$data_path"
