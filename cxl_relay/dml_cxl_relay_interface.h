/*******************
 *dml_cxl_relay_interface.h
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

#ifndef _DML_CXL_RELAY_INTERFACE_H_
#define _DML_CXL_RELAY_INTERFACE_H_

typedef unsigned long long uint64;


#define DML_MAX_DATA_SIZE 256
typedef struct 
{
   uint32_t packet_number;
   uint32_t packet_type;
   uint32_t sim_type;
   uint64_t base_address;
   uint64_t physical_address;
   uint32_t r0w1;
   uint32_t data_size;
   uint8_t  data[DML_MAX_DATA_SIZE];
}DML_SIMICS_PKT_DATA;

int tcp_client_send_bank_access(void *mem_op, void *map_info,int type, void *reply_data);
int dml_send_bank_access_to_cosim(void *mem_op, void *map_info, void *memopdata,int type, void *replydata);

#endif


