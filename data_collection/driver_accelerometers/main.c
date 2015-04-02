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
uint16_t adxl345 [3];
uint16_t itg3200 [3];
uint16_t hmc5843 [3];
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
  uint8_t mac,seq;
  mac = buffer[0];
  seq = buffer[1];
  if (len != 20){
    printf("wrong length:%d\n",len);
    return -1;
  }
  else if  (seq < sequences[mac] && seq != 0){
    printf("sequence seen\n");
    return -1;
  }
  sequences[mac] = seq;
  pkt->mac = mac;
  pkt->seq_no = seq;
  for(i = 2; i < 8; i+=2)
    pkt->adxl345[i-2] =  buffer[i]<<8+ buffer[i+1];
  for(i = 8; i < 14; i+=2)
    pkt->itg3200[i-8] =  buffer[i]<<8+ buffer[i+1];
  for(i = 14; i < 20; i+=2)
    pkt->hmc5843[i-14] = buffer[i]<<8+ buffer[i+1];
  return 0;
}


int main (int argc, char *argv[])
{
  char buffer[MAXBUF];
  int v,cnt,i,p,verbose;
  IMU_PKT pkt;
  time_t ts;
  struct timeval tsub;

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
      printf("v = %d\n",v);
      if (p != -1){
        gettimeofday(&tsub,NULL);
        if (verbose)
        {
          fprintf(data_file,"(timestamp,microseconds,sequence,mac,accX,accY,accZ,gyrX,gyrY,gyrZ,hmcX,hmcY,hmcZ): %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", 
                     tsub.tv_sec,tsub.tv_usec,pkt.seq_no,pkt.mac,
                     pkt.adxl345[0],pkt.adxl345[1],pkt.adxl345[2],
                     pkt.itg3200[0],pkt.itg3200[1],pkt.itg3200[2],
                     pkt.hmc5843[0],pkt.hmc5843[1],pkt.hmc5843[2]);

        }
        else
          fprintf(data_file,"%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", 
                     ts,pkt.seq_no,pkt.mac,
                     pkt.adxl345[0],pkt.adxl345[1],pkt.adxl345[2],
                     pkt.itg3200[0],pkt.itg3200[1],pkt.itg3200[2],
                     pkt.hmc5843[0],pkt.hmc5843[1],pkt.hmc5843[2]);
      }
      //else{
        //fprintf(data_file, "%s\n",buffer);
      //}
    }
  }



}

