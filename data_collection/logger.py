"""
logger.py

This script acts as a supervisor for one or more data collection scripts. The
called scripts can be added to the driver list with optional arguments. Each
called script must be read only (produces no output to disk) or accept 
"""


import os
import subprocess
import time
import Queue
from threading import Thread


class DataDriver:
  """
  An object for collecting the details of a data collection driver. Each driver 
  should have a name, path to the binary, and accept a file base path as an 
  instruction on where to put the data files it generates. This base path can 
  be used to make a folder, name files with suffixes, or generate a .csv file.
  """

  def __init__(self, io_queue, name, exec_path, exec_args, read_only=False):
    """
    Initialize by providing required information.
    name: For identification. Also used to name the data folder
    exec_path: String path to which binary should be called on the command line.
               This binary should accept the data base path as the first arg.
    exec_args: A list of string arguments to be passed after the base path.
    """
    self.io_queue = io_queue
    self.name = name
    self.exec_path = exec_path
    self.exec_args = exec_args
    self.read_only = read_only

    self.subprocess_handle = None
    self.thread_handle = None
    self.newly_ended = False

  def make_data_base_path(self, data_timestamp):
    return "./%s/%s/%s" % (g_data_directory, data_timestamp, self.name)

  # kick off new process execution. should be called only once
  def run(self, data_base_path):
    # compute process string
    if(self.read_only == True):
      process_args = [self.exec_path] + self.exec_args
    else:
      process_args = [self.exec_path] + [data_base_path] + self.exec_args

    # spawn new process 
    self.subprocess_handle = subprocess.Popen(process_args,
                                              stdout=subprocess.PIPE,
                                              stderr=subprocess.STDOUT)
    print("ran %s" % ("\""+"\" \"".join(process_args)+"\""))

    # start monitoring thread
    self.thread_handle = Thread(target=self.feed)
    self.thread_handle.daemon = True # thread dies with the program
    self.thread_handle.start()

  # update function that watches this process. target of per-process thread
  def feed(self):
    while(self.newly_ended == False):
      # fetch new output if the process is running
      for line in iter(self.subprocess_handle.stdout.readline, b""):
        self.queue_print(line)

      # update the process status when finished
      self.subprocess_handle.poll()
      self.queue_print("process died with return code %d" %
                       self.subprocess_handle.returncode)
      self.newly_ended = True

  # push a new line of output onto the logger console
  def queue_print(self, line_to_print):
    self.io_queue.put("%s: %s" % (self.name, line_to_print))


g_io_queue = Queue.Queue()
g_data_directory = "data"
g_driver_list = [
  # DataDriver(g_io_queue, "piksi", "./driver_piksi/driver_piksi", ["/dev/tty.asdf"]),
  # DataDriver(g_io_queue, "accelerometers", "./driver_accelerometers/launch.sh", ["/dev/tty.asdf3"]),
  DataDriver(g_io_queue, "apriltags", "./driver_apriltags/build/bin/apriltags_demo", []),
  # DataDriver(g_io_queue, "test", "./driver_test/a.out", []),
]


def main():
  data_timestamp =  time.ctime()

  # make data folder
  if not os.path.exists(g_data_directory):
    os.mkdir(g_data_directory)
  os.mkdir("/".join([g_data_directory, data_timestamp]))

  # kick off processes
  for driver in g_driver_list:
    driver.run(driver.make_data_base_path(data_timestamp))

  # check in on io queue regularly
  while True:
    try:
      new_line = g_io_queue.get(timeout=.1) # or q.get(timeout=.1)
    except Queue.Empty:
      pass
    else: # got line
      print(new_line.strip("\r\n"))


if __name__ == "__main__":
  main()
