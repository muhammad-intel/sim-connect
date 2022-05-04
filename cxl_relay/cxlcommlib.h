#ifndef __CXLCOMMLIBH__
#define __CXLCOMMLIBH__
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
#else                                    //defined __WINDOWS_COMPILE__
#endif


#define SOCKADDR_IN struct sockaddr_in
#define LPSOCKADDR  struct sockaddr *
int            CXL_GetLastError(void);
int            CXL_InitServerNetwork(int portnumber,int listencount);
int            CXL_CloseClientByServer(int socket);
int            CXL_AcceptClient(int socket,LPSOCKADDR );
int            CXL_CloseSocket(int socket);
unsigned long  CXL_htonl(unsigned long number);
unsigned short CXL_htons(unsigned short number);
int            CXL_Recv(int socket,char *buf,int size,int waitflag);
int            CXL_Send(int socket,char *buf,int size,int waitflag);
int            CXL_InitNetworkLink(unsigned int serveripnumber,int serverportnumber );
void           InvertEndianLong(char *longno);
int            CXL_Connect(int socket);
void           CXL_Delay(int tick);
typedef struct 
{
   short int  hour;
   short int  min;
   short int  sec;
   short  int hundreth;
}CXL_TIME_STRUCT;

typedef struct 
{
   short int        day;
   short int        month;
   short int        year;
   short            dayofweek;
   short int        century;
}CXL_DATE_STRUCT;
char               *GetSystemTime(char *string);
int                 CXL_GetTime(CXL_TIME_STRUCT *ts);
extern int          CXL_server_stopflag;
unsigned long       CXL_htonl(unsigned long no);
unsigned short int  CXL_htons(unsigned short int no);
void                show_connect_error(void);
void                DisplayMessage(char *p);
#define WAIT_FOREVER                   0xFFFFFFFF
#define INVALID_SOCKET                 -1
#define SOCKET_ERROR                   -1
#define ON                             1
#define OFF                            0
#define NOT_CONNECTED                  1
#define CONNECTED                      1
#define CXL_RELAY_FAILURE              -1
#define CXL_RELAY_OK                   0
#define MAX_CLIENT_CONNECT_RETRY_COUNT 1000
#endif

