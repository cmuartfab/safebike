#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <slipstream.h>
#include <time.h>

#define NONBLOCKING  0
#define BLOCKING     1
#define MAXBUF       256


#define MAX_NODES 2

static uint32_t sequences[MAX_NODES];

typedef struct imu_pkt{
uint8_t seq_no;
uint8_t mac;
uint8_t adxl345 [6];
uint8_t itg3200 [6];
uint8_t hmc5843 [6];
} IMU_PKT;

void init()
{
  for (int i = 0; i < MAX_NODES; i++)
  {
    sequences[i] = 0;
  }
}

int unpack(IMU_PKT* pkt, int len, uint8_t* buffer)
{
  int i;
  if (len != 20)
    return -1;
  pkt->seq_no = buffer[0];
  pkt->mac = buffer[1];
  for(i = 2; i < 8; i++)
    pkt->adxl345[i-2] = buffer[i];
  for(i = 8; i < 14; i++)
    pkt->itg3200[i-8] = buffer[i];
  for(i = 14; i < 20; i++)
    pkt->hmc5843[i-14] = buffer[i];
  return 0;
}


int main (int argc, char *argv[])
{
  char buffer[MAXBUF];
  int v,cnt,i,p,verbose;
  IMU_PKT pkt;
  time_t ts;

  verbose = 0;


  // check arguments
  if(argc < 4) {
    printf("[ERROR] Missing arguments. "
           "Example: $ %s data/output/file_base_path server_host server_port\n", argv[0]);
    exit(-1);
  }

  if (argc == 5){
    if (strcmp(argv[4],"-v") == 0){
      printf("Verbose on\n");
      verbose = 1;
    }
  }

  //initialize sequence no to 0.
  init();
  // assign arguments
  const char *data_file_base_path = argv[1];
  char *data_file_path = malloc(strlen(data_file_base_path) + 5);
  sprintf(data_file_path, "%s.csv", data_file_base_path);
  const char *slipstream_host = argv[2];
  const int slipstream_port = atoi(argv[3]);

  // open data file
  FILE *data_file = fopen(data_file_path, "w");
  if(data_file == NULL) {
    printf("[ERROR] Could not open data file %s !\n", data_file_path);
    printf("[ERROR] fopen returned '%s'.\n", strerror(errno));
    exit(-1);
  }

  v=slipstream_open(slipstream_host, slipstream_port, BLOCKING);
  
  while (1) {
    // send first message to let the server know we're here
    sprintf (buffer, "This is a sample slip string.\n");
    v = slipstream_send(buffer, strlen(buffer)+1);

    if (v == 0) {
      printf("[ERROR] Could not send slipstream test message.\n");
      exit(-1);
    }

    v = slipstream_receive(buffer);
    if (v > 0) {
      p = unpack(&pkt,v,buffer);
      if (p != -1){
        ts = time(NULL);
        if (verbose)
        {
          //fprintf(" (time,sequence,mac,
            //         accX,accY,accZ,
              //       gyrX,gyrY,gyrZ,
                //     hmcX,hmcY,hmcZ): 
                  //   %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u");

        }
        fprintf(data_file, "%s\n", buffer);
      }
    }
  }



}

