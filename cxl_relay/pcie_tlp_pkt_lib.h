/*********************
 *PCIe TLP Packet lib
 *********************/
#ifndef _PCIE_TLP_PKT_LIB_H_
#define _PCIE_TLP_PKT_LIB_H_

#include <cxl_tlp_fifo.h>


int simics_print_fifo_pkt_data_and_log(simics_transaction_t *simics_data,FILE *fp,int verbose,int request_reply_flag);
int init_cxl_relay_client_lib(void);	
int init_cxl_relay_server_lib(void);
int init_cxl_relay_lib(void);

int open_server_pipe(void);
int open_client_pipe(void);
int use_named_pipe_set(void);
extern char cxl_relay_log_path[];
int init_log(void);
extern long first_transmit_time;

#endif
