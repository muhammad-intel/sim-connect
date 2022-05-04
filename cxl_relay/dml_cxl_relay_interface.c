/*******************
 *FILE NAME: dml_interface.c
 *
 *
 ***********************/
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
#include "dml_cxl_relay_interface.h"

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
FILE *logfp = NULL;

int packet_number = 0;
int open_relay_log(void)
{
   static char logfile[1024];
   char *p = getenv("CXL_RELAY_LOG_PATH");
   if(p==NULL)
   {
      printf("open_relay_log: CXL_RELAY_LOG_PATH  not set\n");
	  strcpy(cxl_relay_log_path,"./");
      printf("open_relay_log: cxl_relay_log_path set to '%s'\n",cxl_relay_log_path);	  
      return -1;
   }
   strcpy(cxl_relay_log_path,p);
   strcat(cxl_relay_log_path,"/");
   printf("open_relay_log: cxl_relay_log_path set to '%s'\n",cxl_relay_log_path);	     
   sprintf(logfile,"%s%s",cxl_relay_log_path,  "simics_cxl_relay_log.txt"); 
     
   if(logfp==NULL)
      logfp = fopen(logfile,"w+"); 
   if(logfp==NULL)
   {		

         printf("open_relay_log: Coud not open simics_cxl_relay_log.txt, exiting .. \n");
		 exit(0);

   }
	return 0;	
}




