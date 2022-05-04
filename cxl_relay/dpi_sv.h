/*******************
 *dpi_sv.h
 *******************/
#include "svdpi.h"

extern "C" int open_server_fifo(void);
extern "C" int print_tlp_data_simics_request(const int iotype,const char *cxl_data);

extern "C" int  simics_dpi_request
(
   unsigned int       *packet_number,
   unsigned int       *packet_type,
   unsigned int       *sim_type,
   unsigned long long *base_address,
   unsigned long long *physical_address,
   unsigned int       *r0w1,
   unsigned int       *data_size,
   unsigned char      *data
);

extern "C" int  simics_dpi_response
(
   unsigned int packet_number,
   unsigned int sim_type,							
   unsigned int reply_data_size,
   unsigned char *reply_data
);




