
#ifndef __WINDOWS_COMPILE__
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>


#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>
#else                           //defined __WINDOWS_COMPILE__
#endif
#include "cxlcommlib.h"
#include "cxl_tlp_fifo.h"
#include "pcie_tlp_pkt_lib.h"
#include "dml_memaccess.h"


simics_transaction_t test_pkt;       /* request */
simics_transaction_t tlp_data;       /* response */
#define NUM_XACTIONS 10

void
print_timestamp(const char *msg, time_t time, FILE *fp) {
    struct tm *tm;
    if ((tm = localtime (&time)) == NULL) {
        printf ("Error extracting time stuff\n");
        return;
    }
    printf ("%s %04d-%02d-%02d %02d:%02d:%02d\n",
        msg, 
        1900 + tm->tm_year, 
        tm->tm_mon+1, 
        tm->tm_mday,
        tm->tm_hour, 
        tm->tm_min, 
        tm->tm_sec);
    fprintf (fp, "%s %04d-%02d-%02d %02d:%02d:%02d\n",
        msg, 
        1900 + tm->tm_year, 
        tm->tm_mon+1, 
        tm->tm_mday,
        tm->tm_hour, 
        tm->tm_min, 
        tm->tm_sec);
}

int main(int argc, char *argv[])
{
   simics_transaction_t *p;
   unsigned *iptr;
   unsigned j;
   FILE *logfp = NULL;
   int   retval;
   time_t start_t, end_t;       /* simulation performance */


   start_t = time(NULL);
  init_log();
   //print_shared_resources();
   if(open_client_pipe()<0)
   {
      printf("cxl_tlp_relay_client, exit..\n");
      exit(0);		
   }	
   static char logfile[2096];
   sprintf(logfile,"%s%s",cxl_relay_log_path,  "testhost_log.txt"); 
   logfp = fopen(logfile,"w+");
   if(logfp==NULL)
   {
      printf("Can't open testhost_log.txt.. \n");
      return -1;
   }

   j=0;
   for (unsigned i=0; i<NUM_XACTIONS; i++)
   {
      test_pkt.packet_number    = i+1;
      test_pkt.packet_type      = 1; /* 1 for config, 2 for memory */
      test_pkt.sim_type         = 1; /* 0 for simics, 1 for testhost */
      //test_pkt.base_address     = 0x89abcdef00000000;
	  
	  //need to fix
      //test_pkt.physical_address = test_pkt.base_address + 0xe04 + (j << 2); /* Make it byte offset */
      test_pkt.r0w1             = (i & 1);
      if (test_pkt.r0w1 == 0)
         j++;
      test_pkt.data_size        = 4; /* number of bytes */
      iptr                      = (unsigned *) test_pkt.data;
      *iptr                     = 0xfacefeed;

      printf("testhost:  enqueing  the tlp packet to cxl_client fifo..\n");
	 first_transmit_time=(long)time(NULL); 
      simics_print_fifo_pkt_data_and_log(&test_pkt, logfp,1,1);
      p = simics_enque_request_client_tlp(&test_pkt);
      if(p!=NULL)
         printf("testhost: Enquing  successful.\n");
      else
      {
         printf("testhost: Enquing  failed , exiting.. !!\n");
         exit(0);	
      }	   

      /* printf("testhost: Waiting for TLP reply in the fifo..\n"); */
      retval = simics_deque_reply_request_client_tlp(&tlp_data);
      /* printf("testhost: Dequeued the TLP from fifo..\n");	 */
      /* printf("testhost:  received reply from  RTL device ..\n"); */
      simics_print_fifo_pkt_data_and_log(&tlp_data, logfp,1,2);
      /* printf("testhost:  test complete and successful ..\n"); */
   }

   // simulation looks for packet_number == 0 to stop the tes
   printf("testhost:  STOPPNG THE SIMULATION by sending a packet with packet_number==0 ..\n");
   test_pkt.packet_number    = 0;
   p = simics_enque_request_client_tlp(&test_pkt);

   end_t = time(NULL);
   print_timestamp("testhost start", start_t, logfp);
   print_timestamp("testhost end  ", end_t,   logfp);
   printf("Total transaction time= %f seconds,  Time per transaction= %f seconds\n", difftime(end_t, start_t), difftime(end_t, start_t)/NUM_XACTIONS);
   fprintf(logfp, "Total transaction time= %f seconds,  Time per transaction= %f seconds\n", difftime(end_t, start_t), difftime(end_t, start_t)/NUM_XACTIONS);   


   return(p == NULL);           /* should exit with 0, i.e., p != NULL;   1- error */
}
