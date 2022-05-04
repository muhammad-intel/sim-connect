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
#include "cxlcommlib.h"

char PadBuf[256]       = {0};
int Findgetlasterror(void);
int CXLserver_stopflag = 0;
//struct sockaddr client_addr;

SOCKADDR_IN ServerAddr;
//struct sockaddr clientaddr;


#define LPSTR char *

SOCKADDR_IN addr_in;

int             SocketErrorStatus = 0;
#define LISTEN_MAXCLIENT 1 
#define SOCKLEN_T        socklen_t * 
unsigned long   nonblocking       = 0;
int             commdebug         = 0;
unsigned long   CXLhtonl(unsigned long number);
unsigned short  CXLhtons(unsigned short number);
//int show_connect_error          = 0;
int             comm_error;
struct  linger  ling;
struct sockaddr client_addr;

void               DisplayMessage(char *p);
unsigned int       serverip;
unsigned short int serverport;
int                CXL_server_stopflag = 0;
//----------------------------
int CXL_GetLastError(void)
{
   switch(errno)
   {

      case EACCES:
         if(commdebug)
            DisplayMessage((char *) "socket error:'EACCES' \n");
         break;
      case EADDRINUSE:
         if(commdebug)
            DisplayMessage((char *) "socket error:'EADDRINUSE' \n");
         break;
      case EADDRNOTAVAIL:
         if(commdebug)
            DisplayMessage((char *) "socket error:'EADDRNOTAVAL' \n");
         break;
      case EINVAL:
         if(commdebug)
            DisplayMessage((char *) "socket error:'EINVAL' \n");
         break;
      case ENOTSOCK:
         if(commdebug)
            DisplayMessage((char *) "socket error:'ENOTSOCK',The argument s does not refer to a socket. \n");
         break;
      case EBADF:            
         if(commdebug)
            DisplayMessage((char *) "socket error:'EBADF',The argument s is an invalid descriptor. \n");
         break;
      case ENOTCONN:        
         if(commdebug)
            DisplayMessage((char *) "socket error:'ENOTCONN',The socket is associated with a connection-oriented, protocol and has not been                   connected (see connect(2) and accept(2)). \n");
         break;
      case EAGAIN: 
         if(commdebug)
            DisplayMessage((char *) "socket error:'EAGAIN',The socket is marked non-blocking, and the receive,operation would block, or a receive timeout had been,set, and the timeout expired before data were,  received.");
         break;
      case EINTR:
         if(commdebug)
            DisplayMessage((char *) "socket error:'EINTR',The receive was interrupted by delivery of a signal  before any data were available.");
         break;
      case EFAULT:
         if(commdebug)
            DisplayMessage((char *) "socket error:'EFAULT',The receive buffer                                                                                 pointer(s) point outside the process's address space.");
         break;
      default:
         if(commdebug)
            DisplayMessage((char *) "socket error UNKNOWN ERROR CODE' \n");
         break;
   }

   return errno;


}

//------------------------------------------
int CXL_InitServerNetwork(int portnumber,int listencount)
{
   int serversocket = INVALID_SOCKET;
   printf("CXL_InitServerNetwork: portnumber=%d ,listencount = %d\n",portnumber,listencount);

   serversocket = socket(AF_INET,SOCK_STREAM,0);

   if(serversocket==INVALID_SOCKET)
   {
      sprintf(PadBuf,"socket() generated error %d\n",CXL_GetLastError());
      printf(PadBuf);
      return -1;
   }

   //now bind the socket to a port number

   addr_in.sin_family                                           = AF_INET;
   addr_in.sin_port                                             = CXL_htons(portnumber);
   addr_in.sin_addr.s_addr                                      = CXL_htonl(INADDR_ANY);
   if(bind(serversocket,(LPSOCKADDR) &addr_in,sizeof(addr_in)) == SOCKET_ERROR)
   {
      sprintf(PadBuf,"bind() generated error %d\n",CXL_GetLastError());
      printf(PadBuf);
      return -1;
   }
   //now listen for the connection
  
   // if(listen(serversocket,LISTEN_MAXCLIENT) == SOCKET_ERROR)
   if(listen(serversocket,listencount)         == SOCKET_ERROR)
   {
      sprintf(PadBuf,"listen() generated error %d\n",CXL_GetLastError());
      printf(PadBuf);
      return -1;
   }

  
   //ioctlsocket(serversocket,FIONBIO,(unsigned long *) &nonblocking);
   ioctl(serversocket,FIONBIO,(unsigned long *) &nonblocking);

   ling.l_onoff  = 1;
   ling.l_linger = 0;           //all client sockets are hard closed
   printf("CXLInitServerNetwork: server socket initialized\n");
   return serversocket;
}
//----------------------------------
int CXL_CloseClientByServer(int socket)
{
   if(commdebug)
   {
      sprintf(PadBuf,"CXL_CloseClientByServer: Closing Soket No %d ",socket);
      printf(PadBuf);
   }

   setsockopt(socket,SOL_SOCKET,SO_LINGER,(LPSTR)&ling,sizeof(ling));
   // return closesocket(socket);
   return close(socket);
}


//---------------------------------------------
int CXL_AcceptClient(int socket,LPSOCKADDR client_addr )
{
   int clientsocket;
   while(1)
   {
      if(CXL_server_stopflag)
         return 0;
      int clientaddrlen                                                                      = sizeof(struct sockaddr );
      if((clientsocket = accept(socket,(LPSOCKADDR) client_addr,(SOCKLEN_T)&clientaddrlen)) == INVALID_SOCKET)
      {


      }

      CXL_Delay(10);

      if(clientsocket == INVALID_SOCKET) //no connection done
         continue;
  
      return clientsocket;
   }
   return 0;
}
//---------------------------------------------

//---------------------------------------------
int CXL_CloseSocket(int socket)
{
   //return closesocket(socket);
   return close(socket);
}
//---------------------------------------------

unsigned long CXL_htonl(unsigned long number)
{
   return htonl(number);
}
//---------------------------------------------
//---------------------------------------------

unsigned short CXL_htons(unsigned short number)
{
   return htons(number);
}
//////////////////////////////
//---------------------------------------------
int CXL_Send(int socket,char *buf,int size,int waitflag)
{

   long count = 0;

   int ret = 0;


   while((ret = send(socket,buf+count,size-count,0)) >= 1)
   {
      count = count+ret;
      CXL_Delay(1);

   }

   if(!ret)
      return 1;
   CXL_GetLastError();

   return -1;


}
//---------------------------------------------
int CXL_Recv(int socket,char *buf,int size,int waitflag)
{


   long count = 0;
   int  ret;



   switch(waitflag)
   {
      case WAIT_FOREVER:
         while(1)
         {
            if(CXL_server_stopflag)
               return 0;
            ret     = recv(socket,buf,size,0);
            if(ret>0)
               return ret;
            if(ret <= 0)
            {
               return ret;
            }
            CXL_Delay(1);
         }
         break;
      default:
         while(1)
         {

            if(CXL_server_stopflag)
               return 0;

            ret = recv(socket,buf,size,0);
	
            if(ret>0)
               return ret;
            if(ret<0)
               return ret;


            CXL_Delay(1);

	
            if(count >= waitflag)
               return 0;
            ++count;
         }

         break;

   }                            //switch
   return 0;
}


void CXL_Delay(int ms)
{
   usleep( 1000*ms );
}
//////////////
//----------------------
int CXL_InitNetworkLink(unsigned int serveripnumber,int serverportnumber )
{

   //serverport                 = serverportnumber;
   //serverip                   = serveripnumber;
   ServerAddr.sin_family        = AF_INET;
   ServerAddr.sin_port          = CXL_htons(serverportnumber);
   //InvertEndianLong((char *)&serveripnumber);
   ServerAddr.sin_addr.s_addr   = CXL_htonl(serveripnumber);
   //ServerAddr.sin_addr.s_addr = inet_addr("192.168.0.2");
   return 0;

} 

//---------------------------------------------
int CXL_Connect(int socket)
{

   return connect(socket,(LPSOCKADDR)&ServerAddr,sizeof(ServerAddr));
}
//---------------------------------------------
void InvertEndianLong(char *longno)
{
   char *x = longno;
   char  a = *(x);
   char  b = *(x+1);
   char  c = *(x+2);
   char  d = *(x+3);

   *(x)   = d;
   *(x+1) = c;
   *(x+2) = b;
   *(x+3) = a;

}
//------------------------------
char *GetSystemTime(char *string)
{
   CXL_TIME_STRUCT ts;
   CXL_GetTime((CXL_TIME_STRUCT *)&ts);
   sprintf(string,"%02d:%02d:%02d:%02d",ts.hour,ts.min,ts.sec,ts.hundreth);
   return string;
}
//------------------------------
int  CXL_GetTime(CXL_TIME_STRUCT *ts)
{
   struct tm *ntime;
   time_t     aclock;
   time(&aclock);
   ntime    = localtime(&aclock);
   ts->hour     = ntime->tm_hour;
   ts->min      = ntime->tm_min;
   ts->sec      = ntime->tm_sec;
   ts->hundreth = 0;
   return 1;

}
//---------------------

//----------------
void DisplayMessage(char *p)
{
   printf(p);
}

///////////////
void show_connect_error(void)
{
   switch(errno)
   {
      case EACCES:
      case EPERM:
         printf("Write permission is denied on the socket file The user tried to connect to a broadcast address without \
              having the socket broadcast flag enabled\n");
         break;

      case EADDRINUSE:
         printf("Local address is already in use.\n");
         break;
      case EADDRNOTAVAIL:
         printf("(Internet domain sockets) The socket referred to by sockfd \
              had not previously been bound to an address \n");
         break;
      case EAFNOSUPPORT:
         printf("The passed address didn't have the correct address family in its sa_family field.\n");
              
         break;
      case EAGAIN: 
         printf("For nonblocking UNIX domain sockets, the socket is \n \
              nonblocking, and the connection cannot be completed\n \
              immediately.  For other socket families, there are\n \
              insufficient entries in the routing cache.\n");
         break;
      case EALREADY:
         printf("The socket is nonblocking and a previous connection attempt has not yet been completed.\n");
              
         break;
      case EBADF:  printf("sockfd is not a valid open file descriptor.\n");
         break;
      case ECONNREFUSED:
         printf("A connect() on a stream socket found no one listening on the remote address.\n");
              
         break;
      case EFAULT: printf("The socket structure address is outside the user's address space.\n");
              
         break;
      case EINPROGRESS:
         printf("The socket is nonblocking and the connection cannot be completed immediately.\n");
              			  
         break;

      case EINTR:  printf("The system call was interrupted by a signal that was caught\n");
              
         break;
      case EISCONN:
         printf("The socket is already connected.\n");
         break;
      case ENETUNREACH:
         printf("Network is unreachable.\n");
         break;
      case ENOTSOCK:
         printf("The file descriptor sockfd does not refer to a socket.\n");
         break;
      case EPROTOTYPE:
         printf("The socket type does not support the requested communications protocol\n");

         break;
      case ETIMEDOUT:
         printf("Timeout while attempting connection.\n");  
         break;
      default:
         printf("unknown errno = %d, %x\n",errno,errno);  
         break;  	   
   }
}
