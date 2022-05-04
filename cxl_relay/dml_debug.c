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
#include <sys/time.h>
#include <pthread.h>
#include "cxlcommlib.h"
#include "cxl_tlp_fifo.h"
#include "pcie_tlp_pkt_lib.h"
#include "dml_memaccess.h" 

extern FILE *logfp;
void         open_relay_log(void);
//////////////////////////////
int          mem_access_pkt_number = 0;

char *mem_op_type_string(mem_op_type_t type);
char *ini_type_str(ini_type_t ini_type);
char *block_flag_str(block_flag_t block_flag);
long long current_timestamp();
///////////////////////////
int simics_packet_no = 1;
static int time_started=0;
time_t start_t, end_t;       /* simulation performance */
int NUM_XACTIONS=0;
char *mem_op_type_string(mem_op_type_t type)
{
   static char buf[32];
   memset(buf,0,32);
   switch(type)
   {
      case Sim_Trans_Load:
         strcpy(buf,"Sim_Trans_Load");
         break;		
      case Sim_Trans_Store: 
         strcpy(buf,"Sim_Trans_Store");
         break;
      case Sim_Trans_Instr_Fetch:
         strcpy(buf,"Sim_Trans_Instr_Fetch");
         break;		
      case Sim_Trans_Cache:
         strcpy(buf,"Sim_Trans_Cache");
         break;			
      case Sim_Trans_Prefetch:
         strcpy(buf,"Sim_Trans_Prefetch");
         break;
      default:
         strcpy(buf,"Sim_Trans_Unknown");
         break;   		
   }
   return (char *) &buf[0];	
};
/////////////////
static void
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

/////////////////
int simics_send_access_to_cosim(void *send_data,void *reply_data, int verbose, int time_start_flag, int time_end_flag)
{
   simics_transaction_t      *spd = (simics_transaction_t *)send_data;
   simics_transaction_t       *rd = (simics_transaction_t *)reply_data;
   spd->packet_number         = simics_packet_no++;
   open_relay_log();
   if(time_start_flag  && time_started==0)
   {
	   time_started=1;
	   start_t = time(NULL);
	   first_transmit_time=(long)time(NULL); 
   }
   if(time_end_flag)
   {
	   time_started=0;
	   end_t = time(NULL);
	   print_timestamp("testhost start", start_t, logfp);
	   print_timestamp("testhost end  ", end_t,   logfp);
	   printf("Total transaction time= %f seconds,  Time per transaction= %f seconds\n", difftime(end_t, start_t), difftime(end_t, start_t)/NUM_XACTIONS);
	   fprintf(logfp, "Total transaction time= %f seconds,  Time per transaction= %f seconds\n", difftime(end_t, start_t), difftime(end_t, start_t)/NUM_XACTIONS);
	   return 0;
   }      
   if(verbose)
   {
	   fprintf(logfp,"simics_send_access_to_cosim: sending IO packet number %d \n", spd->packet_number);
	   fflush(logfp);
	   printf("simics_send_access_to_cosim: open_client_pipe...\n");
	   fprintf(logfp,"simics_send_access_to_cosim: open_client_pipe\n");
	   fflush(logfp);
	   
   }
   if(open_client_pipe()<0)
   {
	   char *p=getenv("CXL_RELAY_SERVER_PATH");
	   if(p == NULL)
	   {
		  printf("CXL_RELAY_SERVER_PATH id not set\n");
		  fprintf(logfp,"CXL_RELAY_SERVER_PATH id not set\n");
	   }
      printf("simics_send_access_to_cosim, open_client_pipe fails .. exit..\n");
      fprintf(logfp,"simics_send_access_to_cosim, open_client_pipe fails .. CXL_RELAY_SERVER_PATH=%s exit..\n",p);		
      fflush(logfp);
      return -1;		
   }
   long long time_send_req=current_timestamp();
   simics_transaction_t *p = simics_enque_request_client_tlp(spd);
   if(p!=NULL)
   {
	   if(verbose)
	   {	   
		  printf("simics_send_access_to_cosim: Enquing  successful.\n");
		  fprintf(logfp,"simics_send_access_to_cosim: Enquing  successful.\n");		
		  fflush(logfp);
	   }
	   simics_print_fifo_pkt_data_and_log(spd, logfp,verbose,1); 
   }
   else
   {
	   if(verbose)
	   {	   
		  printf("simics_send_access_to_cosim: Enquing failed , exiting.. !!\n");
		  fprintf(logfp,"simics_send_access_to_cosim: Enquing  failed , exiting.. !!\n");		
		  fflush(logfp);
		  exit(0);	
	   }	  
   }	   
   if(verbose)
   {	
	   printf("simics_send_access_to_cosim: Waiting for TLP reply in the fifo..\n");
	   fprintf(logfp,"simics_send_access_to_cosim: Waiting for TLP reply in the fifo..\n");		
	   fflush(logfp);
   }
   int retval = simics_deque_reply_request_client_tlp(rd);
   long long time_recv_reply=current_timestamp();
   if(verbose)
   {   
	   printf("simics_send_access_to_cosim: Dequeued the TLP from fifo..\n");	
	   fprintf(logfp,"simics_send_access_to_cosim: Dequeued the TLP from fifo..\n");		
	   fflush(logfp);
	   printf("simics_send_access_to_cosim:  received reply from  RTL device ..\n");
	   fprintf(logfp,"simics_send_access_to_cosim:  received reply from  RTL device ..\n");	
   }   
   simics_print_fifo_pkt_data_and_log(rd, logfp,verbose,2);  

    printf("simics_send_access_to_cosim: packet_number=%d:  reply request end to end transaction time %lld seconds\n",spd->packet_number,time_recv_reply-time_send_req);
    fprintf(logfp,"simics_send_access_to_cosim: packet_number=%d:  reply request end to end transaction time %lld seconds\n",spd->packet_number,time_recv_reply-time_send_req);
    fflush(logfp);
   if(verbose)
   { 
	   fflush(logfp);
	   printf("simics_send_access_to_cosim:  test complete and successful ..\n");
	   fprintf(logfp,"simics_send_access_to_cosim:  test complete and successful ..\n");		
	   fflush(logfp);
   }
   NUM_XACTIONS++;
   return retval;	   
} 
////////////////////////////
int dml_send_bank_access_to_cosim(void *mem_op, void *map_info,void *memopdata,int type, void *replydata)
{
 

   return 0;	
}
/////////////
long long current_timestamp() 
   {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds/1000;
}