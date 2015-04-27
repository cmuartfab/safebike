"""
logger.py

This script acts as a supervisor for one or more data collection 
"""


import os
import subprocess
import time


class DataDriver:
  """
  An object for collecting the details of a data collection driver. Each driver 
  should have a name, path to the binary, and accept a file base path as an 
  instruction on where to put the data files it generates. This base path can 
  be used to make a folder, name files with suffixes, or generate a .csv file.
  """

  def __init__(self, name, exec_path, exec_args, read_only=False):
    """
    Initialize by providing required information.
    name: For identification. Also used to name the data folder
    exec_path: String path to which binary should be called on the command line.
               This binary should accept the data base path as the first arg.
    exec_args: A list of string arguments to be passed after the base path.
    """
    self.name = name
    self.exec_path = exec_path
    self.exec_args = exec_args
    self.read_only = read_only

    self.subprocess_handle = None
    self.newly_ended = False

  def make_data_base_path(self, data_timestamp):
    return "./%s/%s/%s" % (g_data_directory, data_timestamp, self.name)

  def run(self, data_base_path):
    if(self.read_only == True):
      process_args = [self.exec_path] + self.exec_args
    else:
      process_args = [self.exec_path] + [data_base_path] + self.exec_args
    self.subprocess_handle = subprocess.Popen(process_args,
                                              stdout=subprocess.PIPE,
                                              stderr=subprocess.PIPE)
    print("%s: ran %s" % (self.name, "\""+"\" \"".join(process_args)+"\""))

  def feed(self):
    # update the process status
    self.subprocess_handle.poll()

    # fetch new output if the process is running
    if(self.subprocess_handle.returncode == None):
      self.print_new_info()

    elif(self.newly_ended == False):
      self.newly_ended = True
      print("%s: process died with return code %d" %
            (self.name, self.subprocess_handle.returncode))

  def print_new_info(self):
    # fetch new output
    new_output = self.subprocess_handle.communicate()
    for line in new_output[0].split("\n"):
      print("%s: %s" % (self.name, line))
    for line in new_output[1].split("\n"):
      print("%s: %s" % (self.name, line))


g_data_directory = "data"
g_driver_list = [
  # DataDriver("piksi", "./driver_piksi/driver_piksi", ["/dev/tty.asdf"]),
  # DataDriver("accelerometers", "./driver_accelerometers/launch.sh", ["/dev/tty.asdf3"]),
  DataDriver("apriltags", "./driver_apriltags/build/bin/apriltags_demo", []),
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

  # check in on processes regularly
  while True:
    for driver in g_driver_list:
      driver.feed() 


if __name__ == "__main__":
  main()
