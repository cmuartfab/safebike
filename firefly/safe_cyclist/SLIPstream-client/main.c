#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <slipstream.h>

#define NONBLOCKING  0
#define BLOCKING     1


int main (int argc, char *argv[])
{
  char buffer[128];
  int v,i;

  if (argc != 3) {
    printf ("Usage: server port\n");
    exit (1);
  }

  v=slipstream_open(argv[1],atoi(argv[2]),BLOCKING);
  sprintf (buffer, "Ignore this setup message\n");

    //v=slipstream_acked_send(buffer,strlen(buffer)+1,3);
    v=slipstream_send(buffer,strlen(buffer)+1);
    if (v == 0) printf( "Error sending\n" );
    else printf( "sent pkt\n" );

  
  while (1) {
    v=slipstream_receive( buffer );
    if (v > 0) {
      printf ("Host: ");
      for (i = 0; i < v; i++)
        printf ("%c", buffer[i]);
      printf ("\n");
    }
  }



}

