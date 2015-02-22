#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include "include/libswiftnav/sbp.h"
#include "include/libswiftnav/sbp_messages.h"
#include "include/libswiftnav/sbp_utils.h"

/* Not all OSes have this speed defined */
#if !defined(B1000000)
#define  B1000000 0010010
#endif

#define PIKSI_TTY_PATH "/dev/ttyUSB2"
#define PIKSI_TTY_BAUD B1000000


/* Structs that messages from Piksi will feed. */

static sbp_pos_llh_t      pos_llh;
static sbp_baseline_ned_t baseline_ned;
static sbp_vel_ned_t      vel_ned;
static sbp_dops_t         dops;
static sbp_gps_time_t     gps_time;

/*
 * SBP callback nodes must be statically allocated. Each message ID / callback
 * pair must have a unique sbp_msg_callbacks_node_t associated with it.
 */

static sbp_msg_callbacks_node_t pos_llh_node;
static sbp_msg_callbacks_node_t baseline_ned_node;
static sbp_msg_callbacks_node_t vel_ned_node;
static sbp_msg_callbacks_node_t dops_node;
static sbp_msg_callbacks_node_t gps_time_node;


/* Serial reading handlers */

int open_serial_connection(char *path, speed_t baud_rate) {
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
  if(return_code >= 0) { return_code = cfsetispeed(&toptions, baud_rate); }
  if(return_code >= 0) { return_code = cfsetospeed(&toptions, baud_rate); }

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


uint32_t serial_read(uint8_t *buff, uint32_t n, void *context) {
  uint32_t bytes_read = 0;
  int fd = (int)context; // context pointer is really just fd
  while (bytes_read < n) {
    // do some pointer math so we add to the buffer where we left off
    bytes_read += read(fd, buff + bytes_read, n - bytes_read);
  }
  // printf("serial_read: %un bytes read\n", bytes_read);
  return bytes_read;
}


/*
 * Callback functions to interpret SBP messages.
 * Every message ID has a callback associated with it to
 * receive and interpret the message payload.
 */

void sbp_pos_llh_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
  pos_llh = *(sbp_pos_llh_t *)msg;
}

void sbp_baseline_ned_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
  baseline_ned = *(sbp_baseline_ned_t *)msg;
}

void sbp_vel_ned_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
  vel_ned = *(sbp_vel_ned_t *)msg;
}

void sbp_dops_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
  dops = *(sbp_dops_t *)msg;
}

void sbp_gps_time_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
  gps_time = *(sbp_gps_time_t *)msg;
}


void callback_setup(sbp_state_t *sbp_state_p)
{
  int error; // error checking

  /* Register a node and callback, and associate them with a specific message ID. */
  error = sbp_register_callback(sbp_state_p, SBP_GPS_TIME, &sbp_gps_time_callback, NULL, &gps_time_node);
  printf("sbp_register_callback: %d\n", error);

  error = sbp_register_callback(sbp_state_p, SBP_POS_LLH, &sbp_pos_llh_callback, NULL, &pos_llh_node);
  printf("sbp_register_callback: %d\n", error);
  
  error = sbp_register_callback(sbp_state_p, SBP_BASELINE_NED, &sbp_baseline_ned_callback, NULL, &baseline_ned_node);
  printf("sbp_register_callback: %d\n", error);
  
  error = sbp_register_callback(sbp_state_p, SBP_VEL_NED, &sbp_vel_ned_callback, NULL, &vel_ned_node);
  printf("sbp_register_callback: %d\n", error);
  
  error = sbp_register_callback(sbp_state_p, SBP_DOPS, &sbp_dops_callback, NULL, &dops_node);
  printf("sbp_register_callback: %d\n", error);
}


void callback_data_print(void) {
  // init output string
  /* Use sprintf to right justify floating point prints. */
  char rj[30];
  /* Only want 1 call to SH_SendString as semihosting is quite slow.
   * sprintf everything to this array and then print using array. */
  char str[1000];
  int str_i;
  str_i = 0;
  memset(str, 0, sizeof(str));

  str_i += sprintf(str + str_i, "\n\n\n\n");

  /* Print GPS time. */
  str_i += sprintf(str + str_i, "GPS Time:\n");
  str_i += sprintf(str + str_i, "\tWeek\t\t: %6d\n", (int)gps_time.wn);
  sprintf(rj, "%6.2f", ((float)gps_time.tow)/1e3);
  str_i += sprintf(str + str_i, "\tSeconds\t: %9s\n", rj);
  str_i += sprintf(str + str_i, "\n");

  /* Print absolute position. */
  str_i += sprintf(str + str_i, "Absolute Position:\n");
  sprintf(rj, "%4.10lf", pos_llh.lat);
  str_i += sprintf(str + str_i, "\tLatitude\t: %17s\n", rj);
  sprintf(rj, "%4.10lf", pos_llh.lon);
  str_i += sprintf(str + str_i, "\tLongitude\t: %17s\n", rj);
  sprintf(rj, "%4.10lf", pos_llh.height);
  str_i += sprintf(str + str_i, "\tHeight\t: %17s\n", rj);
  str_i += sprintf(str + str_i, "\tSatellites\t:     %02d\n", pos_llh.n_sats);
  str_i += sprintf(str + str_i, "\n");

  /* Print NED (North/East/Down) baseline (position vector from base to rover). */
  str_i += sprintf(str + str_i, "Baseline (mm):\n");
  str_i += sprintf(str + str_i, "\tNorth\t\t: %6d\n", (int)baseline_ned.n);
  str_i += sprintf(str + str_i, "\tEast\t\t: %6d\n", (int)baseline_ned.e);
  str_i += sprintf(str + str_i, "\tDown\t\t: %6d\n", (int)baseline_ned.d);
  str_i += sprintf(str + str_i, "\n");

  /* Print NED velocity. */
  str_i += sprintf(str + str_i, "Velocity (mm/s):\n");
  str_i += sprintf(str + str_i, "\tNorth\t\t: %6d\n", (int)vel_ned.n);
  str_i += sprintf(str + str_i, "\tEast\t\t: %6d\n", (int)vel_ned.e);
  str_i += sprintf(str + str_i, "\tDown\t\t: %6d\n", (int)vel_ned.d);
  str_i += sprintf(str + str_i, "\n");

  /* Print Dilution of Precision metrics. */
  str_i += sprintf(str + str_i, "Dilution of Precision:\n");
  sprintf(rj, "%4.2f", ((float)dops.gdop/100));
  str_i += sprintf(str + str_i, "\tGDOP\t\t: %7s\n", rj);
  sprintf(rj, "%4.2f", ((float)dops.hdop/100));
  str_i += sprintf(str + str_i, "\tHDOP\t\t: %7s\n", rj);
  sprintf(rj, "%4.2f", ((float)dops.pdop/100));
  str_i += sprintf(str + str_i, "\tPDOP\t\t: %7s\n", rj);
  sprintf(rj, "%4.2f", ((float)dops.tdop/100));
  str_i += sprintf(str + str_i, "\tTDOP\t\t: %7s\n", rj);
  sprintf(rj, "%4.2f", ((float)dops.vdop/100));
  str_i += sprintf(str + str_i, "\tVDOP\t\t: %7s\n", rj);
  str_i += sprintf(str + str_i, "\n");

  printf("%s", str);
}


int main(int argc, char *argv[]) {
  int error; // error checking

  if(argc < 2) {
    printf("[ERROR] Missing arguments. Example: $ %s /dev/tty.something\n", argv[0]);
    exit(-1);
  }

  // open a serial connection to the piksi
  int fd = open_serial_connection(argv[1], PIKSI_TTY_BAUD);
  if(fd < 0) {
    printf("[ERROR] Could not open serial connection %d\n", fd);
    exit(-1);
  }

  // pass in the fd as context
  sbp_state_t s;
  sbp_state_init(&s);
  sbp_state_set_io_context(&s, (void *)fd);

  // setup callback for all messages
  callback_setup(&s);

  while(true) {
    error = sbp_process(&s, &serial_read);
    // if(error != 0) {
    //   printf("sbp_process: %d\n", error);
    // }
    if(error == 1) {
      // new data available
      callback_data_print();
    }
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
