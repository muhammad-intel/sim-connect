#ifndef __DML_MEMACCESSH__
#define __DML_MEMACCESSH__

typedef unsigned long long logical_address_t;
typedef unsigned long long physical_address_t;


typedef unsigned int exception_type_t;
typedef struct _conf_object_
{
   void *init_src;
}conf_object_t ;

typedef struct _transaction_
{
   struct generic_transaction *init_src;
}transaction_t ;	        
enum {
   Sim_Trn_Instr   = 1,
   Sim_Trn_Write   = 2,
   Sim_Trn_Control = 4,


   Sim_Trn_Prefetch = 8
};

typedef enum {
   Sim_Trans_Load           = 0,
   Sim_Trans_Store          = Sim_Trn_Write,
   Sim_Trans_Instr_Fetch    = Sim_Trn_Instr,
   Sim_Trans_Prefetch       = Sim_Trn_Prefetch | Sim_Trn_Control,
   Sim_Trans_Cache          = Sim_Trn_Control
} mem_op_type_t;
typedef enum {
   Sim_Initiator_Illegal    = 0x0,
   Sim_Initiator_CPU        = 0x1000,
   Sim_Initiator_CPU_V9     = 0x1100,
   Sim_Initiator_CPU_X86    = 0x1200,
   Sim_Initiator_CPU_PPC    = 0x1300,
   Sim_Initiator_CPU_Alpha  = 0x1400,
   Sim_Initiator_CPU_MIPS   = 0x1600,
   Sim_Initiator_CPU_ARM    = 0x1700,
   Sim_Initiator_CPU_V8     = 0x1800,
   Sim_Initiator_CPU_H8     = 0x1900,
   Sim_Initiator_CPU_SH     = 0x1a00,
   Sim_Initiator_Device     = 0x2000,
   Sim_Initiator_PCI_Device = 0x2010,
   Sim_Initiator_Cache      = 0x3000,


   Sim_Initiator_Other        = 0x4000,
   Sim_Initiator_User_Defined = 0x5000,


} ini_type_t;
typedef enum {
   Sim_Block_Memhier = 1,

   Sim_Block_MMU       = 2,
   Sim_Block_MMU_IOSTC = 4,


   Sim_Block_Breakpoint    = 8,
   Sim_Block_Notmem        = 16,
   Sim_Block_Endian        = 32,
   Sim_Block_DSTC_Disabled = 64

} block_flag_t;

typedef struct generic_transaction {
   logical_address_t  logical_address;
   physical_address_t physical_address;
   unsigned int       size;
   mem_op_type_t      type;
   unsigned int       atomic:1;


   unsigned int inquiry:1;

   unsigned int non_coherent:1;



   unsigned int ignore:1;



   unsigned int may_stall:1;

   unsigned int reissue:1;


   unsigned int block_STC:1;
   unsigned int use_page_cache:1;

   unsigned int inverse_endian:1;


   unsigned int page_cross:2;




   unsigned int mem_hier_on_fetch:1;


   unsigned int block_flags;


   ini_type_t     ini_type;
   conf_object_t *ini_ptr;

   exception_type_t  exception;
   transaction_t    *transaction;





} generic_transaction_t, MEM_ACCESS;
typedef enum {
   Sim_Addr_Space_Conf,
   Sim_Addr_Space_IO,
   Sim_Addr_Space_Memory
} addr_space_t;
typedef enum {
   Sim_RW_Read        = 0,
   Sim_RW_Write       = 1
} read_or_write_t;
typedef enum {
   Sim_Endian_Target,
   Sim_Endian_Host_From_BE,
   Sim_Endian_Host_From_LE
} endianness_t;
typedef enum swap_mode {
   Sim_Swap_None      = 0,
   Sim_Swap_Bus       = 1,
   Sim_Swap_Bus_Trans = 2,
   Sim_Swap_Trans     = 3
} swap_mode_t;
typedef struct map_info {
   physical_address_t base;
   physical_address_t start;
   physical_address_t length;
   int                function;
   short int          priority;
   int                align_size;
   swap_mode_t        reverse_endian;
} map_info_t;
typedef struct  __pcie_mem_access__
{
   char                  type[8];
   int                   device;
   int                   bus;
   int                   t_type;
   map_info_t            mem_map;
   generic_transaction_t transaction;
}PCIE_MEM_ACCESS;

void cxl_relay_print_mem_access(void *memaccess); 
void cxl_relay_print_mem_access_request(void *memaccess,FILE *logfp);
void cxl_relay_print_memop(void *memop,unsigned long long offset,unsigned long long size,FILE *logfp);
void cxl_relay_dump_access_info(void *memop,unsigned long long offset,unsigned long long size,unsigned int patterns, unsigned int value);
int simics_send_access_to_cosim(void *send_data,void *reply_data, int verbose, int time_start, int time_end);

#endif
