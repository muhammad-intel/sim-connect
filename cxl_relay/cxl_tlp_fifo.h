/**********************
 *  FIFO.H
 *
 *
 ***********************/
#ifndef _CXL_TLP_FIFO_H_
#define _CXL_TLP_FIFO_H_


#define MAX_PAYLOAD_SIZE 256
typedef struct simics_packet
{
   unsigned int       packet_number;
   unsigned int       packet_type;
   unsigned int       sim_type;
   unsigned long long physical_address;
   unsigned int       r0w1;
   unsigned int       data_size;
   unsigned char      data[MAX_PAYLOAD_SIZE];
}simics_transaction_t;


#define SIMICS_PKT_SIZE sizeof(simics_transaction_t)

int init_msg_que();
int open_client_msg_que(void);
int open_server_msg_que(void);
int use_msg_que_set(void);
int print_q_error();

simics_transaction_t *simics_pipe_send_request_client_tlp(simics_transaction_t *simics_data);
int              simics_pipe_receive_request_client_tlp(simics_transaction_t *simics_data);
simics_transaction_t *simics_pipe_send_reply_client_tlp(simics_transaction_t *simics_data);
int              simics_pipe_receive_reply_request_client_tlp(simics_transaction_t *simics_data);
simics_transaction_t *simics_pipe_send_request_server_tlp(simics_transaction_t *simics_data);
int              simics_pipe_receive_request_server_tlp(simics_transaction_t *simics_data);
simics_transaction_t *simics_pipe_send_reply_server_tlp(simics_transaction_t *simics_data);
int              simics_pipe_receive_reply_request_server_tlp(simics_transaction_t *simics_data);

///////////////////
simics_transaction_t *simics_enque_request_client_tlp(simics_transaction_t *simics_data);

int simics_deque_request_client_tlp(simics_transaction_t *simics_data);
simics_transaction_t *simics_enque_reply_client_tlp(simics_transaction_t *simics_data);

int simics_deque_reply_request_client_tlp(simics_transaction_t *simics_data);
simics_transaction_t *simics_enque_request_server_tlp(simics_transaction_t *simics_data);
int simics_deque_request_server_tlp(simics_transaction_t *simics_data);

simics_transaction_t *simics_enque_reply_server_tlp(simics_transaction_t *simics_data);
int simics_deque_reply_request_server_tlp(simics_transaction_t *simics_data);

//extern "C" int init_server_fifo(void);
int init_client_fifo(void);
int init_cxl_relay_client_lib(void);
int init_cxl_relay_server_lib(void);


#endif
