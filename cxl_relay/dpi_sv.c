/*******************
 *dpi_sv.c
 *
 *
 ***********************/
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
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "pcie_tlp_pkt_lib.h"
#include "cxl_tlp_fifo.h"
#include "dml_memaccess.h"
#include "svdpi.h"

FILE *tlp_request_server_file_fp = NULL;
int verbose=0;
int first_packet_received=0;
char cxl_tlp_request_server_text_file_name[1024];

FILE * requestfp = NULL;
FILE * replyfp   = NULL;

/////////////////////////////
extern "C" int open_server_fifo(void)
{
   init_log();	
   static char logfile[1024];   
   sprintf(logfile,"%s%s",cxl_relay_log_path,  "sv_simics_request_log.txt"); 

   
   if (requestfp == NULL)
   {
      requestfp = fopen(logfile,"w+");
      if(requestfp == NULL){
         printf("Couldnt open sv_simics_request_log.txt");
         return -1;
      }
   }
   sprintf(logfile,"%s%s",cxl_relay_log_path,  "sv_simics_reply_log.txt"); 
	
   if (replyfp == NULL)
   {
      replyfp = fopen(logfile,"w+");
      if(replyfp == NULL){
         printf("Couldnt open sv_simics_reply_log.txt");
         return -1;
      }
   }
   printf("open_server_fifo: requestfp=%8x, replyfp=%8x",requestfp,replyfp);
   return open_server_pipe();
}

/////////////////////////////
extern "C" int close_cxl_relay_files(void)
{
   if (requestfp != NULL)
   {
      fclose(requestfp);
      requestfp = NULL;
   }
   if (replyfp != NULL)	
   {
      fclose(replyfp);
      replyfp = NULL;
   }	

   return 0;
}
/////////////////////////////

extern "C" void print_simics_pkt_data(simics_transaction_t *pkt)
{
   simics_print_fifo_pkt_data_and_log(pkt, NULL,0,0);	
}

extern "C" int  simics_dpi_request(simics_transaction_t *to_cosim)
{
   int                    i;
   static simics_transaction_t simics_data;

   printf("dpi_read_simics_request:  enter\n");
   memset((char *) &simics_data,0,sizeof(simics_transaction_t));
   int retval = simics_deque_request_server_tlp(&simics_data);
   if(first_packet_received==0)
   {
	  first_packet_received=1;
	  first_transmit_time=(long)time(NULL); 
   }   
   //printf("simics_dpi_request:  received a SIMICS request from the host, retval=%d\n",retval);
   if (requestfp != NULL)
   {
      //fprintf(requestfp,"simics_dpi_request:  received a SIMICS request from the host, retval = %d\n",retval);
      fflush(requestfp);
   }

   if(retval>0)
   {
      simics_print_fifo_pkt_data_and_log(&simics_data, requestfp,verbose,1);         

      // Pass the simics request to cosim
      to_cosim->packet_number    = simics_data.packet_number;
      to_cosim->packet_type      = simics_data.packet_type;
      to_cosim->sim_type         = simics_data.sim_type;
      to_cosim->physical_address = simics_data.physical_address;
      to_cosim->r0w1             = simics_data.r0w1;
      to_cosim->data_size        = simics_data.data_size;
      memcpy(to_cosim->data, simics_data.data, simics_data.data_size);
		
      printf("simics_dpi_request:  return success\n");
      return 1;
   }
   printf("simics_dpi_request:  received INVALID SIMICS packet from the host\n");
   return -1;	
}
///////////////////////////////////////
extern "C" int  simics_dpi_response(simics_transaction_t *from_cosim)
{
   int i;

   static simics_transaction_t simics_data;
   //printf("simics_dpi_response:  enter\n");
   if(replyfp != NULL)
   {
      //fprintf(replyfp,"simics_dpi_response:  enter\n");
      fflush(replyfp);
   }

   // pass the cosim response to simics
   memset((char *) &simics_data,0,sizeof(simics_transaction_t));
   simics_data.packet_number    = from_cosim->packet_number;
   simics_data.packet_type      = from_cosim->packet_type;
   simics_data.sim_type         = from_cosim->sim_type;
   simics_data.physical_address = from_cosim->physical_address;
   simics_data.sim_type         = from_cosim->sim_type;	
   simics_data.data_size        = from_cosim->data_size;	
   memcpy((char *) &simics_data.data[0], from_cosim->data, from_cosim->data_size);
	
   simics_print_fifo_pkt_data_and_log(&simics_data, replyfp,verbose,2);	

   simics_transaction_t *p  = simics_enque_reply_server_tlp(&simics_data);
   if(p               == NULL)
   {
      printf("simics_dpi_response: SIMICS reply  enqueued to reply fifo failed !! \n");
      if(replyfp != NULL)
      {
         fprintf(replyfp,"simics_dpi_response: SIMICS reply  enqueued to reply fifo failed !! \n");
         fflush(replyfp);
      }
      return -1;
   }
   printf("simics_dpi_response: SIMICS reply  enqueue done!! !! \n");			
   return 1;	
}	
