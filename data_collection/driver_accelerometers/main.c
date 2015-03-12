#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <slipstream.h>

#define NONBLOCKING  0
#define BLOCKING     1
#define MAXBUF       256


int main (int argc, char *argv[])
{
  char buffer[MAXBUF];
  int v,cnt,i;

  // check arguments
  if(argc < 4) {
    printf("[ERROR] Missing arguments. "
           "Example: $ %s data/output/file_base_path server_host server_port\n", argv[0]);
    exit(-1);
  }
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
    if(v > 0) {
      fprintf(data_file, "%s\n", buffer);
    }
  }



}

