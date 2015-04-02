#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <slipstream.h>

#define NONBLOCKING  0
#define BLOCKING     1

#define MAX_NODES 2

static uint32_t sequences[MAX_NODES];

typedef struct imu_pkt{
uint8_t seq_no;
uint8_t mac;
uint8_t adxl345 [6];
uint8_t itg3200 [6];
uint8_t hmc5843 [6];
}

void init()
{
  for (int i = 0; i < MAX_NODES; i++)
  {
    sequence[i] = 0;
  }
}

imu_pkt unpack(int len, uint8_t* buffer)
{
  if (len != 20)
    return NULL;
  imu_pkt pkt = malloc(sizeof(imu_pkt));
  pkt.seq_no = imu_pkt[0];
  pkt.mac = imu_pkt[1];
  for(int i = 2; i < 8; i++)
    pkt.adxl345[i-2] = buffer[i];
  for(i = 8; i < 14; i++)
    pkt.itg3200[i-8] = buffer[i];
  for(i = 14; i < 20; i++)
    pkt.hmc5843[i-14] = buffer[i];
  return pkt;
}


int main (int argc, char *argv[])
{
  char buffer[128];
  int v,i;
  imu_pkt pkt;

  if (argc != 3) {
    printf ("Usage: server port\n");
    exit (1);
  }



  v=slipstream_open(argv[1],atoi(argv[2]),BLOCKING);
  sprintf (buffer, "Ignore this setup message\n");

    v=slipstream_send(buffer,strlen(buffer)+1);
    if (v == 0) printf( "Error sending\n" );
    else printf( "sent pkt\n" );

  
  while (1) {
 v=slipstream_send(buffer,strlen(buffer)+1);
    v=slipstream_receive( buffer );
    if (v > 0) {
      printf ("Host: ");
      for (i = 0; i < v; i++)
        printf ("%c", buffer[i]);
      printf ("\n");
     pkt = unpack(v,buffer);
     if (pkt != NULL){
        
     }
    }

   
  }



}

