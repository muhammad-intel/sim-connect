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
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pcie_tlp_pkt_lib.h"
#include "cxl_tlp_fifo.h"
#include "dml_memaccess.h"

int request_client_pipe;
int reply_client_pipe;
int request_server_pipe;
int reply_server_pipe;

char          request_client_pipe_str[1024];
char          reply_client_pipe_str[1024];
char          request_server_pipe_str[1024];
char          reply_server_pipe_str[1024];
struct pollfd request_server_pipe_pfd;
struct pollfd reply_server_pipe_pfd;
int           init_server_fifo(void);
int           init_client_fifo(void);
int init_client_fifo(void)
{
   char *p = getenv("CXL_RELAY_SERVER_PATH");
   if(p==NULL)
   {
      printf("CXL_RELAY_SERVER_PATH id not set\n");
      return -1;
   }	

   sprintf(request_client_pipe_str,"%s/%s",p,"request_client_pipe");
   sprintf(reply_client_pipe_str,"%s/%s",p,"reply_client_pipe");
   sprintf(request_server_pipe_str,"%s/%s",p,"request_server_pipe");
   sprintf(reply_server_pipe_str,"%s/%s",p,"reply_server_pipe");
   printf("CXL_RELAY_SERVER_PATH=%s\n",p);	
   printf("request_client_pipe_str=%s",reply_server_pipe_str);	
   printf("reply_client_pipe_str=%s",reply_server_pipe_str);	
   printf("request_server_pipe_str=%s",reply_server_pipe_str);	
   printf("reply_server_pipe_str=%s",reply_server_pipe_str);	
   request_client_pipe = mkfifo(request_client_pipe_str, 0777); 
                  
   if(request_client_pipe<0)
   {
      if(errno!=EEXIST)
      {
         print_q_error();
         printf("init_named_fifo: Can't create request_client_pipe !!\n");
         return -1;
      }
   }
   reply_client_pipe = mkfifo(reply_client_pipe_str, 0777);
   if(reply_client_pipe<0)
   {
      if(errno!=EEXIST)
      {		 
         close( request_client_pipe);
         print_q_error();
         printf("init_named_fifo: Can't create reply_client_pipe !!\n");
         return -1;
      }
   }
   return 0;
}	
//////////////////////
int init_server_fifo(void)
{	
   char *p = getenv("CXL_RELAY_SERVER_PATH");
   if(p    == NULL)
   {
      printf("CXL_RELAY_SERVER_PATH id not set\n");
      return -1;
   }	
   sprintf(request_client_pipe_str,"%s/%s",p,"request_client_pipe");
   sprintf(reply_client_pipe_str,"%s/%s",p,"reply_client_pipe");
   sprintf(request_server_pipe_str,"%s/%s",p,"request_server_pipe");
   sprintf(reply_server_pipe_str,"%s/%s",p,"reply_server_pipe");
   printf("CXL_RELAY_SERVER_PATH = %s\n",p);	
   printf("request_client_pipe_str=%s",reply_server_pipe_str);	
   printf("reply_client_pipe_str=%s",reply_server_pipe_str);	
   printf("request_server_pipe_str=%s",reply_server_pipe_str);	
   printf("reply_server_pipe_str=%s",reply_server_pipe_str);	
   request_server_pipe = mkfifo(request_server_pipe_str,  0777);
   if(request_server_pipe<0)
   {
      if(errno != EEXIST)
      {		 
         close( request_client_pipe);
         close( reply_client_pipe);
         print_q_error();
         printf("init_named_fifo: Can't create request_server_pipe !!\n");
         return -1;
      }
   }
   reply_server_pipe = mkfifo(reply_server_pipe_str, 0777);
   if(reply_server_pipe<0)
   {
      if(errno != EEXIST)
      {			 
         close( request_client_pipe);
         close( reply_client_pipe);
         close( request_server_pipe);			 
         print_q_error();
         printf("init_named_fifo: Can't create reply_server_pipe !!\n");
      }
      return -1;
   }	
   return 0;	 
}
 
//////////////////////
int init_named_fifo(void)
{	
   char *p = getenv("CXL_RELAY_SERVER_PATH");
   if(p == NULL)
   {
      printf("CXL_RELAY_SERVER_PATH id not set\n");
      return -1;
   }	
   sprintf(request_client_pipe_str,"%s/%s",p,"request_client_pipe");
   sprintf(reply_client_pipe_str,"%s/%s",p,"reply_client_pipe");
   sprintf(request_server_pipe_str,"%s/%s",p,"request_server_pipe");
   sprintf(reply_server_pipe_str,"%s/%s",p,"reply_server_pipe");
   printf("CXL_RELAY_SERVER_PATH=%s\n",p);	
   printf("request_client_pipe_str=%s",reply_server_pipe_str);	
   printf("reply_client_pipe_str=%s",reply_server_pipe_str);	
   printf("request_server_pipe_str=%s",reply_server_pipe_str);	
   printf("reply_server_pipe_str=%s",reply_server_pipe_str);	
   request_client_pipe = mkfifo(request_client_pipe_str, 0777); 
                  
   if(request_client_pipe<0)
   {
      if(errno != EEXIST)
      {
         print_q_error();
         printf("init_named_fifo: Can't create request_client_pipe !!\n");
         return -1;
      }
   }
   reply_client_pipe = mkfifo(reply_client_pipe_str, 0777);
   if(reply_client_pipe<0)
   {
      if(errno != EEXIST)
      {		 
         close( request_client_pipe);
         print_q_error();
         printf("init_named_fifo: Can't create reply_client_pipe !!\n");
         return -1;
      }
   }
   request_server_pipe=mkfifo(request_server_pipe_str,  0777);
   if(request_server_pipe<0)
   {
      if(errno != EEXIST)
      {		 
         close( request_client_pipe);
         close( reply_client_pipe);
         print_q_error();
         printf("init_named_fifo: Can't create request_server_pipe !!\n");
         return -1;
      }
   }
   reply_server_pipe=mkfifo(reply_server_pipe_str, 0777);
   if(reply_server_pipe<0)
   {
      if(errno != EEXIST)
      {			 
         close( request_client_pipe);
         close( reply_client_pipe);
         close( request_server_pipe);			 
         print_q_error();
         printf("init_named_fifo: Can't create reply_server_pipe !!\n");
      }
      return -1;
   }	
   return 0;	 
}
////////////////
int close_named_fifo(void)
{
   close( request_client_pipe);
   close( reply_client_pipe);
   close( request_server_pipe);
   close( reply_server_pipe);

   return 0;
}
//////////////////////
int open_client_pipe(void)
{
   char *p=getenv("CXL_RELAY_SERVER_PATH");
   if(p == NULL)
   {
      printf("CXL_RELAY_SERVER_PATH id not set\n");
      return -1;
   }	
   sprintf(request_client_pipe_str,"%s/%s",p,"request_client_pipe");
   sprintf(reply_client_pipe_str,"%s/%s",p,"reply_client_pipe");
   sprintf(request_server_pipe_str,"%s/%s",p,"request_server_pipe");
   sprintf(reply_server_pipe_str,"%s/%s",p,"reply_server_pipe");
   printf("CXL_RELAY_SERVER_PATH = %s\n",p);	
   printf("request_client_pipe_str = %s\n",reply_server_pipe_str);	
   printf("reply_client_pipe_str   = %s\n",reply_server_pipe_str);	
   printf("request_server_pipe_str = %s\n",reply_server_pipe_str);	
   printf("reply_server_pipe_str   = %s\n",reply_server_pipe_str);	
   request_client_pipe=open(request_client_pipe_str, O_RDWR);
   if(request_client_pipe<0)
   {
      print_q_error();
      printf("open_client_pipe: Can't create request_client_pipe !!\n");
      return -1;
   }
   reply_client_pipe = open(reply_client_pipe_str, O_RDWR );
   if(reply_client_pipe<0)
   {
      close( request_client_pipe);
      print_q_error();
      printf("open_client_pipe: Can't create reply_client_pipe !!\n");
      return -1;
   }	

   return 0;
}
/////////////////
int open_server_pipe(void)
{
   char *p=getenv("CXL_RELAY_SERVER_PATH");
   if(p == NULL)
   {
      printf("CXL_RELAY_SERVER_PATH id not set\n");
      return -1;
   }	
   sprintf(request_client_pipe_str,"%s/%s",p,"request_client_pipe");
   sprintf(reply_client_pipe_str,"%s/%s",p,"reply_client_pipe");
   sprintf(request_server_pipe_str,"%s/%s",p,"request_server_pipe");
   sprintf(reply_server_pipe_str,"%s/%s",p,"reply_server_pipe");
   printf("CXL_RELAY_SERVER_PATH = %s\n",p);	
   printf("request_client_pipe_str=%s",reply_server_pipe_str);	
   printf("reply_client_pipe_str=%s",reply_server_pipe_str);	
   printf("request_server_pipe_str=%s",reply_server_pipe_str);	
   printf("reply_server_pipe_str = %s",reply_server_pipe_str);		
   request_server_pipe           = open(request_server_pipe_str, O_RDWR);
   if(request_server_pipe<0)
   {
      print_q_error();
      printf("open_server_pipe: Can't open request_server_pipe !!\n");
      return -1;
   }
	 

   //memset(&request_server_pipe_pfd,0,sizeof(struct pollfd));
   //request_server_pipe_pfd.fd     = request_server_pipe;
   //request_server_pipe_pfd.events = POLLIN;
	
   reply_server_pipe=open(reply_server_pipe_str, O_RDWR);
   if(reply_server_pipe<0)
   {
      close( request_server_pipe);			 
      print_q_error();
      printf("open_server_pipe: Can't open reply_server_pipe !!\n");
      return -1;
   }

   //memset(&reply_server_pipe_pfd,0,sizeof(struct pollfd));
   //reply_server_pipe_pfd.fd     = reply_server_pipe;
   //reply_server_pipe_pfd.events = POLLIN;	 
   //CXL_TLP_DATA tlp_data;
   //printf("open_server_pipe: removing old requests !!\n");	 
   //while(pipe_receive_reply_request_server_tlp(&tlp_data)>0);
   //while(pipe_receive_request_server_tlp(&tlp_data)>0);
   //printf("open_server_pipe: removal of old requests done!!\n");

   return 0;
}
/////////////////
int close_client_pipe(void)
{

   if(request_client_pipe>=0)
      close( request_client_pipe);
   if(reply_client_pipe >= 0)
      close( reply_client_pipe);

   return 0;
	 
}
/////////////////

/////////////////
int print_q_error()
{
   switch( errno)
   {
      case EACCES: 
         printf("Msg q error: EACCES\n");
         break;
      case EEXIST: 
         printf("Msg q error: EEXIST\n");
         break;		
      case EINVAL:  
         printf("Msg q error: EINVAL\n");
         break;					
      case EMFILE:
         printf("Msg q error: EMFILE\n");
         break;		
      case ENAMETOOLONG: 
         printf("Msg q error: ENAMETOOLONG\n");
         break;
      case ENFILE:
         printf("Msg q error: ENFILE\n");
         break;		
      case ENOENT:
         printf("Msg q error: ENOENT\n");
         break;					
      case ENOMEM:
         printf("Msg q error: ENOMEM\n");
         break;		
      case ENOSPC:
         printf("Msg q error: ENOSPC\n");
         break;	
      default:
         printf("Msg q error: no %d \n",errno);
         break;	
		 
   }

   return 0;
}

/////////////
/////////////////
simics_transaction_t *simics_pipe_send_request_client_tlp(simics_transaction_t *simics_data)
{
   int retval = write( 
      request_client_pipe ,     //mqd_t             mqdes, 
      (const char *) simics_data, //const char *msg_ptr, 
      sizeof(simics_transaction_t)
   );	
   if(retval==-1)
   {
      printf("send_request_client_tlp: could not send the TLP request to clinet\n");
      return NULL;
   }
   return simics_data;
}
///////////////////////////////
int simics_pipe_receive_request_client_tlp(simics_transaction_t *simics_data)
{
   int                                          size = read( request_client_pipe, //mqd_t mqdes, 
                                                             (char *) simics_data, //char *msg_ptr, 
                                                             sizeof(simics_transaction_t) 
   );
   return size;						
}
///////////////////////////////
simics_transaction_t *simics_pipe_send_reply_client_tlp(simics_transaction_t *simics_data)
{
   int retval = write( 
      reply_client_pipe ,       //mqd_t               mqdes, 
      (const char *) simics_data, //const char *msg_ptr, 
      sizeof(simics_transaction_t)
   );	
   if(retval==-1)
   {
      printf("send_reply_client_tlp: could not send the TLP reply to client reply queue\n");
      return NULL;
   }
   return simics_data;	
}
///////////////////////////////
int simics_pipe_receive_reply_request_client_tlp(simics_transaction_t *simics_data)
{
   int                                          size = read( reply_client_pipe, //mqd_t mqdes, 
                                                             (char *) simics_data, //char *msg_ptr, 
                                                             sizeof(simics_transaction_t) 
   );
   return size;		
}
///////////////////////////////
simics_transaction_t *simics_pipe_send_request_server_tlp(simics_transaction_t *simics_data)
{
   printf("send_request_server_tlp: simics_data = %llx\n",(long long unsigned int)simics_data);
   int retval                                   = write( 
      request_server_pipe ,     //mqd_t mqdes, 
      (const char *) simics_data, //const char *msg_ptr, 
      sizeof(simics_transaction_t)
   );	
   if(retval<=0)
   {
      printf("send_request_server_tlp: could not send the TLP requrest to DPI queue\n");
      return NULL;
   }
   printf("send_request_server_tlp: write TLP to the queue is successful, simics_data = %llx\n",(long long unsigned int)simics_data);
   return simics_data;	
}
///////////////////////////////
int simics_pipe_receive_request_server_tlp(simics_transaction_t *simics_data)
{
   int size = -1;
   //if(reply_server_pipe_has_data())
   {
      size = read( request_server_pipe, //mqd_t  mqdes, 
                   (char *) simics_data, //char *msg_ptr, 
                   sizeof(simics_transaction_t) 
      );
   }
   return size;	
}
///////////////////////////////
simics_transaction_t *simics_pipe_send_reply_server_tlp(simics_transaction_t *simics_data)
{
   int retval = write( 
      reply_server_pipe ,       //mqd_t              mqdes, 
      (const char *) simics_data, //const char *msg_ptr, 
      sizeof(simics_transaction_t)
   );	
   if(retval  == -1)
   {
      printf("send_reply_server_tlp: could not send the TLP reply to transport server queue\n");
      return NULL;
   }
   return simics_data;	
}
///////////////////////////////
int simics_pipe_receive_reply_request_server_tlp(simics_transaction_t *simics_data)
{
   int size = -1;
   //if(reply_server_pipe_has_data())
   {
      size = read( reply_server_pipe, //mqd_t    mqdes, 
                   (char *) simics_data, //char *msg_ptr, 
                   sizeof(simics_transaction_t) 
      );
   }
   return size;		
}
///////////////////////////////
