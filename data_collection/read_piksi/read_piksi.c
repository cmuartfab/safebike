#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "libswiftnav/sbp.h"
#include "libswiftnav/sbp_messages.h"
#include "libswiftnav/sbp_utils.h"


int open_serial_connection(char *path) {
  int return_code = 0;
  int fd = 0;
  
  // open unix-style file with termios
  return_code = open(path, O_RDWR | O_NONBLOCK);
  if(!isatty(return_code)) {
    printf("connectionOpen: %s is not a tty!", path);
    close(return_code);
  } else {
    fd = return_code;
  }
  
  // proceed with setting up serial connection. abort if any function returns 
  // error (return_code < 0)
  struct termios toptions;

  if(return_code >= 0) { return_code = tcgetattr(fd, &toptions); }
  if(return_code >= 0) { return_code = cfsetispeed(&toptions, B115200); }
  if(return_code >= 0) { return_code = cfsetospeed(&toptions, B115200); }

  // 8N1
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  // no flow control
  toptions.c_cflag &= ~CRTSCTS;

  //toptions.c_cflag &= ~HUPCL; // disable hang-up-on-close to avoid reset

  toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
  toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

  toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
  toptions.c_oflag &= ~OPOST; // make raw

  // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
  toptions.c_cc[VMIN]  = 0;
  toptions.c_cc[VTIME] = 0;
  //toptions.c_cc[VTIME] = 20;

  if(return_code >= 0) { return_code = tcsetattr(fd, TCSANOW, &toptions); }
  if(return_code >= 0) { return_code = tcsetattr(fd, TCSAFLUSH, &toptions); }  
  
  if(return_code < 0) {
    return return_code;
  } else {
    return fd;
  }
}

uint32_t serial_read(uint8_t *buff, uint32_t n, void *context)
{
  uint32_t bytes_read = 0;
  int fd = (int)context; // context pointer is really just fd
  while (bytes_read < n) {
    // do some pointer math so we add to the buffer where we left off
    bytes_read += read(fd, buff + bytes_read, n - bytes_read);
  }
  return bytes_read;
}

int main() {
  // open a serial connection to the piksi
  char *path = "/dev/tty.usbserial-00001014";
  int fd = open_serial_connection(path);
  if(fd < 0) {
    printf("Could not open serial connection: %n\n", fd);
    exit(-1);
  }

  // pass in the fd as context
  sbp_state_t s;
  sbp_state_init(&s);
  sbp_state_set_io_context(&s, (void *)fd);
  while(true) {
    sbp_process(&s, &serial_read);
  }

  // serial test program
  // uint8_t sample_buffer[256];
  // uint32_t i = 0;
  // while(1) {
  //   serial_read(&sample_buffer, 256, (void *)fd);
  //   printf("read %d samples\n", i);
  //   i++;
  // }

}