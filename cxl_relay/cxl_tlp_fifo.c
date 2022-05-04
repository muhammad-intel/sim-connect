/**********************
 *  cxl_tlp_fifo.c
 **********************/
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
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>


#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cxl_tlp_fifo.h"
#include "dml_memaccess.h"

int use_msg_que    = 0;
int use_named_pipe = 1;
int use_named_pipe_set(void)
{
   return use_named_pipe;
}
///////////////////////

simics_transaction_t *simics_enque_request_client_tlp(simics_transaction_t *simics_data)
{

   if(use_named_pipe)
   {
     // printf("enque_request_client_tlp: use_named_pipe..\n");	
      return simics_pipe_send_request_client_tlp(simics_data);		
   }

   return NULL;
}	
//////////////////////
int simics_deque_request_client_tlp(simics_transaction_t *simics_data)
{

   if(use_named_pipe)
   {
      //printf("simics_deque_request_client_tlp: use_named_pipe..\n");	
      return simics_pipe_receive_request_client_tlp(simics_data);		
   }	
	
   return 0;
}
///////////////


////////////////
simics_transaction_t *simics_enque_reply_client_tlp(simics_transaction_t *simics_data)
{

 
   if(use_named_pipe)
   {
      //printf("simics_enque_reply_client_tlp: use_named_pipe..\n");	
      return simics_pipe_send_reply_client_tlp(simics_data);		
   }	

   return NULL;
}
/////////////////

int simics_deque_reply_request_client_tlp(simics_transaction_t *simics_data)
{

   if(use_named_pipe)
   {
      //printf("simics_deque_reply_request_client_tlp: use_named_pipe..\n");	
      return simics_pipe_receive_reply_request_client_tlp(simics_data);		
   }	
	
   return 0;
}
///////////////

//server side funcs

////////////////
simics_transaction_t *simics_enque_request_server_tlp(simics_transaction_t *simics_data)
{


   if(use_named_pipe)
   {
      //printf("simics_enque_request_server_tlp: use_named_pipe..\n");	
      return simics_pipe_send_request_server_tlp(simics_data);		
   }	
	
   return NULL;
}

//////////////////////
int simics_deque_request_server_tlp(simics_transaction_t *simics_data)
{
	

   if(use_named_pipe)
   {
      //printf("simics_deque_request_server_tlp: use_named_pipe..\n");	
      return simics_pipe_receive_request_server_tlp(simics_data);		
   }	
	
   return 0;
}
///////////////


simics_transaction_t *simics_enque_reply_server_tlp(simics_transaction_t *simics_data)
{


   if(use_named_pipe)
   {
      //printf("simics_enque_reply_server_tlp: use_named_pipe..\n");	
      return simics_pipe_send_reply_server_tlp(simics_data);		
   }	

   return NULL;
}

//////////////////////
int simics_deque_reply_request_server_tlp(simics_transaction_t *simics_data)
{

   if(use_named_pipe)
   {
      //printf("simics_deque_reply_request_server_tlp: use_named_pipe..\n");	
      return simics_pipe_receive_reply_request_server_tlp(simics_data);		
   }	
	
   return 0;
}
///////////////
