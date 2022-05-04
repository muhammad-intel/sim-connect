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
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#else                           //defined __WINDOWS_COMPILE__
#endif
#include "cxlcommlib.h"
#include "cxl_tlp_fifo.h"
#include "pcie_tlp_pkt_lib.h"
#include "dml_memaccess.h"
#define CXL_RELAY_FAILURE -1
#define CXL_RELAY_OK      0
int	 usenetwork=0;
int  rtl_side_tcp_port               = 1979;
int  cxl_tcp_server_socket           = -1;
int  server_stopflag                 = 0;
int  cxl_tcp_server_connection_state = NOT_CONNECTED;
int  spawn_reply_back_thread(void * param);
void UpdateCxlTlpDebugWatch(void);

int   DumpForSystemVerilogFifoInPut(simics_transaction_t *tlp_data);
FILE *logfp      = NULL;
int   logging_on = 0;

void       simics_log_cxl_packet(FILE *logfp,simics_transaction_t *tlp_data,int verbose,int flag);
int        rtl_side_tcp_server_init(void);
void      *cxl_tcp_server_thread(void *parameter);
void *cxl_server_fifo_access_thread(void *parameter);
void      *cxl_rtl_to_host_reply_thread(void *parameter);

pthread_t  server_threadId;
int        stop_reply_thread    = 0;
int        total_tlp_reply_sent = 0;
int cxl_server_stopflag=0;
int stop_request_read_thread=0;
int verbose=0;
int first_packet_received=0;
int main(int argc, char *argv[])
{
   int retval = init_cxl_relay_server_lib();	

   printf("cxl_tlp_relay_server, version 1.0\n");
char           *serverportenv = getenv("CXL_RELAY_SERVER_PORT"); 
   if(serverportenv == NULL)
   {
      printf("CXL_RELAY_SERVER_PORT id not set\n");
      return -1;
   } 
   rtl_side_tcp_port=(int)atoi(serverportenv);
   char           *cxlrelayoverip = getenv("CXL_RELAY_OVER_IP"); 
   if(cxlrelayoverip == NULL)
   {
      printf("CXL_RELAY_OVER_IP id not set\n");
      return -1;
   }  
   if(strcmp( cxlrelayoverip,"yes") ==0)
    usenetwork=1;
    else
	 usenetwork=0;	   
   init_log();
   static char logfile[1024];
   sprintf(logfile,"%s%s",cxl_relay_log_path,  "cxl_relay_server_log.txt"); 
   logfp = fopen(logfile,"w+");   
   if(logfp == NULL)
   {
      printf("Can't open cxl_relay_server_log.txt.. \n");
      return -1;
   }	
   //print_shared_resources();
   if(open_server_pipe()<0)
   {
      printf("cxl_tlp_relay_server, exit..\n");
      exit(0);		
   }	
  if(argc>1)
   verbose=  (strcmp(argv[1],"-v")==0) ? 1 :0;

   if(rtl_side_tcp_server_init() == CXL_RELAY_FAILURE)
   {
      printf("Could not start cxl_tlp_relay_server. Exiting ...\n");
   }

   pthread_join(server_threadId, NULL);
   printf("cxl_tlp_relay_server, exit..\n");	
}
///////////////////////////////////
int rtl_side_tcp_server_init(void)
{
   //pthread_attr_t attr = NULL;
   //initialize the server socket and listener
   int retval =-1;
   if(usenetwork==1)
   {
	   cxl_tcp_server_socket = CXL_InitServerNetwork(rtl_side_tcp_port,1);
	   if(cxl_tcp_server_socket<0)
	   {
		  return CXL_RELAY_FAILURE;
	   }
	   //spawn the cxl relay server thread 


	   retval  = pthread_create(&server_threadId, 
									NULL,
									&cxl_tcp_server_thread, 
									(void *)NULL);
   }
   else
   {
	   	   retval  = pthread_create(&server_threadId, 
									NULL,
									&cxl_server_fifo_access_thread, 
									(void *)NULL);
   }
   if(retval  != CXL_RELAY_OK)
      return 	CXL_RELAY_FAILURE;  
   return CXL_RELAY_OK;							  
}

///////////////////////
void *cxl_tcp_server_thread(void *parameter)
{

   static char            systime[256];
   static simics_transaction_t simics_data;
   int                    ret;
   int                    cxl_clientsock = -1;
   static struct sockaddr clientsock;
   int                    total_cxl_pkt_received            = 0;
   printf("cxl_tcp_server_thread spawned ...\n");

   server_stopflag                  = 0;
   cxl_tcp_server_connection_state  = NOT_CONNECTED;
   while(server_stopflag           == OFF)
   {
      usleep(1000);
      if(server_stopflag)
         break;
      printf("cxl_tcp_server_thread: waiting for client connection...\n");
      cxl_clientsock = CXL_AcceptClient(cxl_tcp_server_socket,&clientsock);
      if(cxl_clientsock>0)
      {
         printf("cxl_tcp_server_thread:connected to client\n");			
         //update connection state
         cxl_tcp_server_connection_state                                              = CONNECTED;
         stop_reply_thread                                                            = 0;
         //spawn a reply back thread here
         printf("cxl_tcp_server_thread:Spawning reply back thread with cxl_clientsock = %d\n",cxl_clientsock);
         spawn_reply_back_thread((void *) &cxl_clientsock);
         UpdateCxlTlpDebugWatch();
         while(server_stopflag                                                        == 0)
         {

            ret = CXL_Recv(cxl_clientsock,(char *) &simics_data,SIMICS_PKT_SIZE,WAIT_FOREVER);
            if(ret == SIMICS_PKT_SIZE)
            {
               GetSystemTime(systime);
							
               total_cxl_pkt_received++;
			   if(verbose)
               printf("cxl_tcp_server_thread: total_cxl_pkt_received from host = %d, pktsize=%d, time =%s \n",total_cxl_pkt_received,ret,systime);

               if(logfp != NULL)
               {
                  if(logging_on)
                  {
                     simics_print_fifo_pkt_data_and_log(&simics_data,logfp,verbose,1);
                     //fprintf(logfp,"Received message : %s\n",CommandBuf);
                     //fflush(logfp);
                  }
               }
               
			   //if(verbose)
               //printf("cxl_tcp_server_thread: writing request to text file '%s' ..\n",tlp_request_rtl_side_file_name);
               //DumpForSystemVerilogTextInPut(&simics_data);
			   if(verbose)			   
               printf("cxl_tcp_server_thread: writing request to binary fifo ..\n");
               DumpForSystemVerilogFifoInPut(&simics_data);
               UpdateCxlTlpDebugWatch();
            }
            else
            {
			   if(verbose)	
               printf("cxl_tcp_server_thread: closing client connection ..\n");
               CXL_CloseSocket(cxl_clientsock);
               cxl_clientsock                  = -1;
               cxl_tcp_server_connection_state = NOT_CONNECTED;
               UpdateCxlTlpDebugWatch();
               stop_reply_thread               = 0;
               break;
            }
         }                      //while receive
         if(server_stopflag)
         {
			if(verbose) 
            printf("cxl_tcp_server_thread: server_stopflag = 1 quiting ..\n");
            CXL_CloseSocket(cxl_clientsock);
            cxl_clientsock                                 = -1;
            cxl_tcp_server_connection_state                = NOT_CONNECTED;
            stop_reply_thread                              = 1;

         }
      }                         //if connected
      stop_reply_thread = 1;
   }                            //while server flag
   printf("cxl_relay_server_thread Stopped\n");

   return NULL;
}
///////////////////////
int spawn_reply_back_thread(void * param)
{
   pthread_t threadId;
   int       retval  = pthread_create(&threadId, 
                                      NULL,
                                      &cxl_rtl_to_host_reply_thread, 
                                      (void *)param);	
   if(retval        != CXL_RELAY_OK)
      return 	CXL_RELAY_FAILURE;  
   return CXL_RELAY_OK;
}

//////////////////////////
void *cxl_rtl_to_host_reply_thread(void *parameter)
{
   printf("cxl_rtl_to_host_reply_thread:  started..\n");
   int                    cxl_clientsock;
   if(usenetwork)
   cxl_clientsock= (int ) *((int *) parameter);
   static simics_transaction_t simics_data;
   int                    wait_fifo_printf_flag=0;
   while(1)
   {
      usleep(200);
      if(stop_reply_thread)
         break;
      if(server_stopflag)
         break;
      if(wait_fifo_printf_flag == 0)
		  if(verbose)
         printf("cxl_rtl_to_host_reply_thread: checking retply from RTL device in the reply fifo ..\n");	
      if(!use_named_pipe_set())
      {	
         //if(reply_server_fifo_empty())
         {
            ++wait_fifo_printf_flag;
            CXL_Delay(100);
            continue;
         }
         wait_fifo_printf_flag = 0;
		 if(verbose)
         printf("cxl_rtl_to_host_reply_thread: Received RTL reply, dequeing the request\n");	
      }		
      int retval = simics_deque_reply_request_server_tlp(&simics_data);
      if(verbose)
      printf("cxl_rtl_to_host_reply_thread:  received relpy from the RTL. Sending reply back to the HOST\n");
      if(usenetwork)
      {	  
		int success = CXL_Send(cxl_clientsock,(char *)&simics_data,SIMICS_PKT_SIZE,WAIT_FOREVER);
		if(success  == 1) 
		  {
			 total_tlp_reply_sent++;
			 simics_print_fifo_pkt_data_and_log(&simics_data,logfp,verbose,2);
			 if(verbose)		 
			 printf("cxl_rtl_to_host_reply_thread: total_tlp_reply_sent = %d\n",total_tlp_reply_sent);
		  }
		  else
         printf("cxl_rtl_to_host_reply_thread: sending tlp_reply to host failed\n");
	  }
	  else
	  {
		   if(verbose)
            printf("cxl_rtl_to_host_reply_thread: Enquing  TLP reply to host fifo\n");
            simics_transaction_t *p=simics_enque_reply_client_tlp(&simics_data);
			if(p==NULL)
			{
              printf("cxl_rtl_to_host_reply_thread: sending tlp_reply to host failed\n");
			}
			else
			{
			 total_tlp_reply_sent++;
			 simics_print_fifo_pkt_data_and_log(&simics_data,logfp,verbose,2);
			 if(verbose)		 
			 printf("cxl_rtl_to_host_reply_thread: total_tlp_reply_sent = %d\n",total_tlp_reply_sent);
			}		 

	  }
      	
   }
   if(verbose)
   printf("cxl_rtl_to_host_reply_thread: exit: stop_reply_thread = %d, server_stopflag=%d\n",stop_reply_thread,server_stopflag);
   return NULL;
}
//////////////////////////
void UpdateCxlTlpDebugWatch(void)
{
}

/////////////////////////
void simics_log_cxl_packet(FILE *logfp,simics_transaction_t *simics_data,int verbose,int flag)
{
   simics_print_fifo_pkt_data_and_log(simics_data,logfp,verbose,flag);
}
////////////////////////////
int DumpForSystemVerilogTextInPut(simics_transaction_t *simics_data)
{
   //if(verbose)	
   //printf("DumpForSystemVerilogTextInPut: Writing TLP PACKET to file '%s'\n",tlp_request_rtl_side_file_name);
   return 0;
}
////////////////////////////
int DumpForSystemVerilogFifoInPut(simics_transaction_t *simics_data)
{
   if(verbose)	
   printf("DumpForSystemVerilogFifoInPut: Writing TLP PACKET to binary fifo ..\n");	
   simics_transaction_t *p                        = simics_enque_request_server_tlp(simics_data);
   if(verbose)   
   printf("DumpForSystemVerilogFifoInPut: p  = %x \n",p);
   if(p == NULL)
   {
      printf("Could not put the TLP in the RTL queue\n");
	  return 0;
   }
   return 1;
}
////////////////////////////
///////////////////////
void *cxl_server_fifo_access_thread(void *parameter)
{
	printf("cxl_server_fifo_access_thread:  started..\n");
	static simics_transaction_t simics_data;
	int total_tlp_request_sent=0;
	int wait_fifo_printf_flag=0;
   if(open_client_pipe()<0)
   {
      printf("cxl_server_fifo_access_thread: Failed to open Simulator Host side fifo, exit..\n");
      exit(0);		
   }
   spawn_reply_back_thread(parameter)  ; 
   while(1)
   {
	if(cxl_server_stopflag)
         exit(0);	
      if(stop_request_read_thread)
        exit(0);	
      if(wait_fifo_printf_flag == 0)
		 if(verbose)
         printf("cxl_server_fifo_access_thread: checking TLP request from host in the request fifo ..\n");
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
         printf("cxl_server_fifo_access_thread: TLP request found in the fifo..\n");
      }

      int retval = simics_deque_request_client_tlp(&simics_data);
	  if(verbose)
      printf("cxl_server_fifo_access_thread: received the TLP from fifo..\n");
	  if(first_packet_received==0)
	   {
		  first_packet_received=1;
		  first_transmit_time=(long)time(NULL); 
	   } 
      simics_print_fifo_pkt_data_and_log(&simics_data,logfp,verbose,1);
	  if(verbose)
      printf("cxl_server_fifo_access_thread: sending host tlp request to RTL side server..\n");
      int success  = DumpForSystemVerilogFifoInPut(&simics_data);

      if(success )
      {	
         if(verbose)
         printf("cxl_server_fifo_access_thread: send successful..\n");	
         total_tlp_request_sent++;
		 if(verbose)
         printf("cxl_server_fifo_access_thread: total_tlp_request_sent = %d\n",total_tlp_request_sent);
      }
      else
         printf("cxl_server_fifo_access_thread: sending tlp to the server failed \n");			

   }
   if(verbose)
   printf("cxl_server_fifo_access_thread: exit: stop_request_read_thread = %d, cxl_server_stopflag=%d\n",stop_request_read_thread,cxl_server_stopflag);
   return NULL;
}