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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#else                           //defined __WINDOWS_COMPILE__
#endif
#include "cxlcommlib.h"
#include "cxl_tlp_fifo.h"
#include "pcie_tlp_pkt_lib.h"
//FILE * requestfp                              = NULL;
int           rtl_side_tcp_port                 = 1979;
int           host_side_tcp_client_init();
char          tlp_request_host_side_file_name[1024];
void         *cxl_relay_client_thread(void *ptr);
pthread_t     client_threadId;
int           cxl_client_stopflag               = 0;
int           stop_request_read_thread          = 0;
int           cxl_relay_client_connection_state = NOT_CONNECTED;
unsigned int  cxl_relay_server_ip               = 0x7f000001;
int           cxl_relay_server_port             = 1979;
int usenetwork=0;
int   simics_validate_receive(simics_transaction_t *tlp_data);
int   spawn_host_request_read_thread(void *param);
int   total_cxl_reply_received = 0;
int   total_tlp_request_sent   = 0;
FILE *logfp                    = NULL;
int verbose=0;
int first_packet_received=0;
int main(int argc, char *argv[])
{
   struct in_addr  inp;
   char           *serverip = getenv("CXL_RELAY_SERVER_IP"); 
   if(serverip == NULL)
   {
      printf("CXL_RELAY_SERVER_IP id not set\n");
      return -1;
   }
   char           *serverportenv = getenv("CXL_RELAY_SERVER_PORT"); 
   if(serverportenv == NULL)
   {
      printf("CXL_RELAY_SERVER_PORT id not set\n");
      return -1;
   } 
   cxl_relay_server_port=(int)atoi(serverportenv);
   char           *cxlrelayoverip = getenv("CXL_RELAY_OVER_IP"); 
   if(cxlrelayoverip == NULL)
   {
      printf("CXL_RELAY_OVER_IP id not set\n");
      return -1;
   }  
   if(strcmp( cxlrelayoverip,"yes") ==0)
    usenetwork=1;
    else
	{
	 usenetwork=0;
	 printf("No network in use. cxl_tlp_relay_client:  exit the program.\n");
	 exit(0);
	}	 
   init_log();
   static char logfile[1024];
   sprintf(logfile,"%s%s",cxl_relay_log_path,  "cxl_relay_client_log.txt"); 
   logfp = fopen(logfile,"w+");
   if(logfp==NULL)
   {
      printf("cxl_tlp_relay_client: Can't open cxl_relay_client_log.txt.. \n");
      return -1;
   }
   if(argc>1)
   verbose=  (strcmp(argv[1],"-v")==0) ? 1 :0;
   strcpy(tlp_request_host_side_file_name,"tlp_request_host_side_file_name.txt");
   printf("cxl_tlp_relay_client, version 1.0\n");
   printf("cxl_tlp_relay_client, default ip = '%08x, environment var serverip='%s', serverport=%d\n",cxl_relay_server_ip,serverip,cxl_relay_server_port);
   int inetret         = inet_aton((const char *)serverip, &inp);
   cxl_relay_server_ip = ntohl(inp.s_addr);
   printf("cxl_tlp_relay_client: inp.s_addr = %08x, cxl_relay_server_ip=%08X\n",inp.s_addr,cxl_relay_server_ip);
	
   int retval = init_cxl_relay_client_lib();	
   if(open_client_pipe()<0)
   {
      printf("cxl_tlp_relay_client, exit..\n");
      exit(0);		
   }
   //print_shared_resources();
   if(argc>1)
   {
      strcpy(tlp_request_host_side_file_name,argv[1]);
   }
   printf("tlp_request_host_side_file_name ='%s' \n",tlp_request_host_side_file_name);
   if(host_side_tcp_client_init() == CXL_RELAY_FAILURE)
   {
      printf("Could not start cxl_tlp_relay_client. Exiting ...\n");
   }

   pthread_join(client_threadId, NULL);
   printf("cxl_tlp_relay_client, exit..\n");	
}
///////////////////////////////////

///////////////////////

int host_side_tcp_client_init(void)
{

   //spawn the cxl relay client thread 

   int retval = pthread_create(&client_threadId, 
                               NULL,
                               &cxl_relay_client_thread, 
                               NULL);
   if(retval  != CXL_RELAY_OK)
      return 	CXL_RELAY_FAILURE;  
   return CXL_RELAY_OK;
}

///////////////////////



////////////////////////////////	
void *cxl_relay_client_thread(void *ptr)
{

   static char            systime[256];
   int                    connected_to_server = -1;
   int                    connect_retry_count = 0;
   int                    net_init;
   int                    cxl_relay_client_socket;
   cxl_client_stopflag                        = 0;
   cxl_relay_client_connection_state          = NOT_CONNECTED;
   int                    success;
   static simics_transaction_t simics_data;
   printf("cxl_relay_client_thread spawned\n");




   net_init = CXL_InitNetworkLink(cxl_relay_server_ip,cxl_relay_server_port);
   if(net_init)
   { 
      return NULL;
   }
   cxl_relay_client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
   printf("cxl_relay_client_thread:cxl_relay_client_socket = %x\n",cxl_relay_client_socket);
   if(cxl_relay_client_socket<0)
   {
      printf("cxl_relay_client_thread: network initialization failed\n");
      return NULL;
   }

   while(cxl_client_stopflag==OFF)
   {

      printf("cxl_relay_client_thread:Trying to connect to the server..\n");
      connected_to_server = CXL_Connect(cxl_relay_client_socket);
		
      if(connected_to_server == SOCKET_ERROR)
      {
         ++connect_retry_count;
         printf("cxl_relay_client_thread:Trying to connect to the server %d\n",connect_retry_count);
				
         if(connect_retry_count> MAX_CLIENT_CONNECT_RETRY_COUNT)
         {
            //printf("cxl_relay_client_thread:Tried to connect to server %d times, aborting..\n",connect_retry_count);
            //stop_request_read_thread = 1;	
            //return NULL;
         }
         usleep(1000);
         continue;
      }
      connect_retry_count = 0;
      printf("cxl_relay_client_thread:connected_to_server  = %d\n",connected_to_server);
      while(cxl_client_stopflag                           == OFF)
      {
			 
         if(connected_to_server      >= 0)
         {
				
            //spawn the thread that will read host requests and send to the relay server
            //if(!read_host_request_thread_spawned)
            stop_request_read_thread  = 0;	
            printf("cxl_relay_client_thread:spawning host request fifo read thread\n");
            spawn_host_request_read_thread((void *)&cxl_relay_client_socket);
			
            //update connection state
            cxl_relay_client_connection_state = CONNECTED;
			  
         }
			
         while(cxl_client_stopflag == 0) //wait for reply
         {
					
						
            int ret = CXL_Recv(cxl_relay_client_socket,(char *)&simics_data,SIMICS_PKT_SIZE,WAIT_FOREVER);
			if(verbose)
            printf("cxl_relay_client_thread: recived TLP reply from the server, receive length  = %d\n",ret);
            if(ret  == SIMICS_PKT_SIZE)
            {
               GetSystemTime(
                  systime
               );
               total_cxl_reply_received++; 
			   if(verbose)
               printf("cxl_relay_client_thread: total_cxl_reply_received: %d\n",total_cxl_reply_received);
               if(simics_validate_receive(&simics_data) <= 0)
               {
                  CXL_CloseSocket(cxl_relay_client_socket);
                  cxl_relay_client_connection_state = NOT_CONNECTED;
                  connected_to_server               = -1;
                  stop_request_read_thread          = 1;
                  break;
               }
               else
               {
                  //write to reply fifo
				  if(verbose)
                  printf("cxl_relay_client_thread: Enquing  TLP reply to host fifo\n");
                  simics_transaction_t *p=simics_enque_reply_client_tlp(&simics_data);
				  
				
					  if(p != NULL)
					  {
						 if(verbose)
						 printf("cxl_relay_client_thread: Enquing  successful.\n");
					     simics_print_fifo_pkt_data_and_log(&simics_data,logfp,verbose,2);
					  }
					  else
						 printf("cxl_relay_client_thread: Enquing  failed !!\n");
				
								
               }

							
            }
            else
            {
			  if(verbose)				
               printf("cxl_relay_client_thread: Receive error closing the connection and reconnect\n");
               CXL_CloseSocket(cxl_relay_client_socket);
               CXL_Delay(2000);
               cxl_relay_client_socket           = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
               cxl_relay_client_connection_state = NOT_CONNECTED;
               connected_to_server               = -1;
               stop_request_read_thread          = 1;
               break;   

            }
				

					
         }                      //cxl_client_stopflag == 0
         if(connected_to_server<0)
            break;

      }                         //connected loop
      if(cxl_client_stopflag)
      {
		 if(verbose)		  
         printf("cxl_relay_client_thread: cxl_client_stopflag = 1. thread exit\n");
         CXL_CloseSocket(cxl_relay_client_socket);
         cxl_relay_client_socket                              = -1;
         cxl_relay_client_connection_state = NOT_CONNECTED;
         stop_request_read_thread          = 1;
         return NULL;
      }		
   }
   if(verbose)
   printf("cxl_relay_client_thread: thread stopped!!\n");
   return NULL;
}
/////////////////////////
void *host_request_read_thread(void *p)
{
   int                    cxl_relay_server_server_socket = *((int *)p);
   static simics_transaction_t simics_data;
   int                    wait_fifo_printf_flag          = 0;
   if(verbose)
   printf("host_request_read_thread: client side:  started..\n");
   //read from the fifo
   while(1)
   {
      if(cxl_client_stopflag)
         break;
      if(stop_request_read_thread)
         break;
      if(wait_fifo_printf_flag == 0)
		 if(verbose)
         printf("host_request_read_thread: checking TLP request from host in the request fifo ..\n");
      if(!use_named_pipe_set())
      {
         //if(request_client_fifo_empty())
         {
            wait_fifo_printf_flag++;
            CXL_Delay(100);
            continue;
         }
         wait_fifo_printf_flag = 0;
		 if(verbose) 
         printf("host_request_read_thread: TLP request found in the fifo..\n");
      }

      int retval = simics_deque_request_client_tlp(&simics_data);
	  if(verbose)
      printf("host_request_read_thread: received the TLP from fifo..\n");
	  if(first_packet_received==0)
	   {
		  first_packet_received=1;
		  first_transmit_time=(long)time(NULL); 
	   } 
      simics_print_fifo_pkt_data_and_log(&simics_data,logfp,verbose,1);
	  if(verbose)
      printf("host_request_read_thread: sending host tlp request to RTL side server..\n");
      int success  = CXL_Send(cxl_relay_server_server_socket,(char *)&simics_data,SIMICS_PKT_SIZE,WAIT_FOREVER);
      if(success == 1)
      {	
         if(verbose)
         printf("host_request_read_thread: send successful..\n");	
         total_tlp_request_sent++;
		 if(verbose)
         printf("host_request_read_thread: total_tlp_request_sent = %d\n",total_tlp_request_sent);
      }
      else
         printf("host_request_read_thread: sending tlp to the server failed \n");			

   }
   if(verbose)
   printf("host_request_read_thread: exit: stop_request_read_thread = %d, cxl_client_stopflag=%d\n",stop_request_read_thread,cxl_client_stopflag);
   return NULL;
}
/////////////////////////
int spawn_host_request_read_thread(void *param)
{
   pthread_t        threadId;
   //pthread_attr_t attr;
   int              retval  = pthread_create(&threadId, 
                                             NULL,
                                             &host_request_read_thread, 
                                             (void *)param);
   if(retval               != CXL_RELAY_OK)
      return 	CXL_RELAY_FAILURE;  
   return CXL_RELAY_OK;							  
}

////////////
int simics_validate_receive(simics_transaction_t *tlp_data)
{
   return 1;
}
