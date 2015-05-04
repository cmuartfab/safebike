# Copyright (C) 2015 Swift Navigation Inc.
# Contact: Fergus Noble <fergus@swiftnav.com>
#
# This source is subject to the license found in the file 'LICENSE' which must
# be be distributed together with this source. All other rights reserved.
#
# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
# EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

"""
the :mod:`sbp.client.examples.simple` module contains a basic example of
reading SBP messages from a serial port, decoding BASELINE_NED messages and
printing them out.
"""

from sbp.client.drivers.pyserial_driver import PySerialDriver
from sbp.client.handler import Handler
import sbp.navigation, sbp.acquisition
import argparse
import time
import sys
from datetime import datetime, timedelta


FILENAME_GPS_TIME = "_gps_time.csv"
FILENAME_DOPS = "_dops.csv"
FILENAME_POS_ECEF = "_pos_ecef.csv"
FILENAME_POS_LLH = "_pos_llh.csv"
FILENAME_BASELINE_ECEF = "_baseline_ecef.csv"
FILENAME_BASELINE_NED = "_baseline_ned.csv"
# FILENAME_VEL_ECEF = "_vel_ecef.csv"
# FILENAME_VEL_NED = "_vel_ned.csv"


class SharedState:
  file_base_path = None
  file_gps_time = None
  file_dops = None
  file_pos_ecef = None
  file_pos_llh = None
  file_baseline_ecef = None
  file_baseline_ned = None
  # file_vel_ecef = None
  # file_vel_ned = None

  flag_have_solution = False


def check_file(file_pointer, file_name):
  if(file_pointer == None):
    print("bad file pointer %s!" % file_name)
    sys.stdout.flush()
    sys.exit(-1)


def cb_gps_time(msg, shared_state):
  data_file = shared_state.file_gps_time
  check_file(data_file, "g_file_gps_time")
  # set the first solution flag
  if(not shared_state.flag_have_solution):
    print("Found first solution.")
    sys.stdout.flush()
  shared_state.flag_have_solution = True

  m = sbp.navigation.MsgGPSTime(msg)
  timestamp = get_time_us()
  data_file.write("%d, %d, %d, %d, %d\n" % \
    (m.wn, m.tow, m.ns, timestamp[0], timestamp[1]))


def cb_dops(msg, shared_state):
  data_file = shared_state.file_dops
  check_file(data_file, "g_file_dops")
  m = sbp.navigation.MsgDops(msg)
  data_file.write("%d, %d, %d, %d, %d, %d\n" % \
    (m.tow, m.gdop, m.pdop, m.tdop, m.hdop, m.vdop))


def cb_pos_ecef(msg, shared_state):
  data_file = shared_state.file_pos_ecef
  check_file(data_file, "g_file_pos_ecef")
  m = sbp.navigation.MsgPosECEF(msg)
  data_file.write("%d, %d, %d, %d, %d, %d, %d\n" % \
    (m.tow, m.x, m.y, m.z, m.accuracy, m.n_sats, m.flags))


def cb_pos_llh(msg, shared_state):
  data_file = shared_state.file_pos_llh
  check_file(data_file, "g_file_pos_llh")
  m = sbp.navigation.MsgPosLLH(msg)
  data_file.write("%d, %d, %d, %d, %d, %d, %d, %d\n" % \
    (m.tow, m.lat, m.lon, m.height, m.h_accuracy, m.v_accuracy, m.n_sats, m.flags))


def cb_baseline_ecef(msg, shared_state):
  data_file = shared_state.file_baseline_ecef
  check_file(data_file, "g_file_baseline_ecef")
  m = sbp.navigation.MsgBaselineECEF(msg)
  data_file.write("%d, %d, %d, %d, %d, %d\n" % \
    (m.tow, m.x, m.y, m.z, m.accuracy, m.n_sats))


def cb_baseline_ned(msg, shared_state):
  data_file = shared_state.file_baseline_ned
  check_file(data_file, "g_file_baseline_ned")
  m = sbp.navigation.MsgBaselineNED(msg)
  data_file.write("%d, %d, %d, %d, %d, %d, %d\n" % \
    (m.tow, m.n, m.e, m.d, m.h_accuracy, m.v_accuracy, m.n_sats))


def cb_acq_result(msg, shared_state):
  m = sbp.acquisition.MsgAcqResult(msg)
  # check_file(data_file, "g_file_gps_time")
  # data_file.write("Searching for satellites... SNR of %d for PRN %d.\n" % \
  #     (m.snr, m.prn))
  if(not shared_state.flag_have_solution):
    print("Searching for satellites... SNR of %d for PRN %d." % \
      (m.snr, m.prn))
    sys.stdout.flush()


# use the datetime function to get microsecond-precise local system time.
# returns tuple with seconds since epoch and microsecond residual
def get_time_us():
  now = datetime.now()
  epoch = datetime(1970, 1, 1) # use POSIX epoch
  posix_timestamp = (now - epoch)
  timestamp_s = (posix_timestamp.days * 3600) + posix_timestamp.seconds
  timestamp_residual_us = posix_timestamp.microseconds
  return (timestamp_s, timestamp_residual_us)


def closure1(fn, arg1):
  def closure_target(msg):
    fn(msg, arg1)
  return closure_target


def main():
  shared_state = SharedState()

  # parse input args
  parser = argparse.ArgumentParser(description="Swift Navigation SBP Example.")
  parser.add_argument("-p", "--port",
                      default=['/dev/ttyUSB0'], nargs=1,
                      help="specify the serial port to use.")
  parser.add_argument("-d", "--data-base-path",
                      default=[None], nargs=1,
                      help="specify the location data should be recorded to.")
  args = parser.parse_args()

  # example time
  timestamp_example = get_time_us()
  print("System time is %ds + %dus." % (timestamp_example[0], timestamp_example[1]))
  sys.stdout.flush()

  # try to open data collection files
  if(args.data_base_path != None):
    print "Using data base path \"%s\"." % args.data_base_path[0]
    shared_state.file_base_path = args.data_base_path[0].strip()
    sys.stdout.flush()

    shared_state.file_gps_time = open(shared_state.file_base_path + FILENAME_GPS_TIME, 'w')
    shared_state.file_dops = open(shared_state.file_base_path + FILENAME_DOPS, 'w')
    shared_state.file_pos_ecef = open(shared_state.file_base_path + FILENAME_POS_ECEF, 'w')
    shared_state.file_pos_llh = open(shared_state.file_base_path + FILENAME_POS_LLH, 'w')
    shared_state.file_baseline_ecef = open(shared_state.file_base_path + FILENAME_BASELINE_ECEF, 'w')
    shared_state.file_baseline_ned = open(shared_state.file_base_path + FILENAME_BASELINE_NED, 'w')
    # shared_state.file_vel_ecef = open(shared_state.file_base_path + FILENAME_VEL_ECEF, 'w')
    # shared_state.file_vel_ned = open(shared_state.file_base_path + FILENAME_VEL_NED, 'w')

    # check opening files... because sometimes this doesn't work
    if(shared_state.file_gps_time == None):
      print("Data file opening failed!")
      sys.stdout.flush()
      sys.exit(-1)

    # write csv file headers
    shared_state.file_gps_time.write("week number (week), time of week (ms), residual (ns), system time (s), system time residual (us)\n")
    shared_state.file_dops.write("tow (ms), gdop, pdop, tdop, hdop, vdop\n")
    shared_state.file_pos_ecef.write("tow (ms), x (mm), y (mm), z (mm), accuracy (mm), n_sats, flags\n")
    shared_state.file_pos_llh.write("tow (ms), latitude (deg), longitude (deg), height (m), h_accuracy (mm), v_accuracy (mm), n_sats, flags\n")
    shared_state.file_baseline_ecef.write("tow (ms), x (mm), y (mm), z (mm), accuracy (mm), n_sats\n")
    shared_state.file_baseline_ned.write("tow (ms), n (mm), e (mm), d (mm), h_accuracy (mm), v_accuracy (mm), n_sats\n")
    # shared_state.file_vel_ecef.write("tow (ms), x (mm/s), y (mm/s), z (mm/s), accuracy (mm/s), n_sats\n")
    # shared_state.file_vel_ned.write("tow (ms), n (mm/s), e (mm/s), d (mm/s), h_accuracy (mm/s), v_accuracy (mm/s), n_sats\n")


  # Open a connection to Piksi using the default baud rate (1Mbaud)
  with PySerialDriver(args.port[0], baud=1000000) as driver:
    # Create a handler to connect our Piksi driver to our callbacks
    with Handler(driver.read, driver.write, verbose=True) as handler:
      # Add a callback for BASELINE_NED messages
      handler.add_callback(closure1(cb_gps_time, shared_state), msg_type=sbp.navigation.SBP_MSG_GPS_TIME)
      handler.add_callback(closure1(cb_dops, shared_state), msg_type=sbp.navigation.SBP_MSG_DOPS)
      handler.add_callback(closure1(cb_pos_ecef, shared_state), msg_type=sbp.navigation.SBP_MSG_POS_ECEF)
      handler.add_callback(closure1(cb_pos_llh, shared_state), msg_type=sbp.navigation.SBP_MSG_POS_LLH)
      handler.add_callback(closure1(cb_baseline_ecef, shared_state), msg_type=sbp.navigation.SBP_MSG_BASELINE_ECEF)
      handler.add_callback(closure1(cb_baseline_ned, shared_state), msg_type=sbp.navigation.SBP_MSG_BASELINE_NED)
      handler.add_callback(closure1(cb_acq_result, shared_state), msg_type=sbp.acquisition.SBP_MSG_ACQ_RESULT)
      handler.start()

      # Sleep until the user presses Ctrl-C
      try:
        while True:
          time.sleep(0.1)
      except KeyboardInterrupt:
        pass

if __name__ == "__main__":
  main()