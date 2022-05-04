#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <dlfcn.h>
#include "svdpi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _VC_TYPES_
#define _VC_TYPES_
/* common definitions shared with DirectC.h */

typedef unsigned int U;
typedef unsigned char UB;
typedef unsigned char scalar;
typedef struct { U c; U d;} vec32;

#define scalar_0 0
#define scalar_1 1
#define scalar_z 2
#define scalar_x 3

extern long long int ConvUP2LLI(U* a);
extern void ConvLLI2UP(long long int a1, U* a2);
extern long long int GetLLIresult();
extern void StoreLLIresult(const unsigned int* data);
typedef struct VeriC_Descriptor *vc_handle;

#ifndef SV_3_COMPATIBILITY
#define SV_STRING const char*
#else
#define SV_STRING char*
#endif

#endif /* _VC_TYPES_ */

typedef struct _vcs_dpi_simics_transaction_t	simics_transaction_t;

struct	_vcs_dpi_simics_transaction_t	{
	unsigned int	packet_number;
	unsigned int	packet_type;
	unsigned int	sim_type;
	unsigned long long	base_address;
	unsigned long long	physical_address;
	unsigned int	r0w1;
	unsigned int	data_size;
	unsigned char	data[256];
};



 extern int open_server_fifo();

 extern void print_simics_pkt_data(const /* INPUT */simics_transaction_t *pkt);

 extern int simics_dpi_request(/* OUTPUT */simics_transaction_t *from_simics);

 extern int simics_dpi_response(const /* INPUT */simics_transaction_t *to_simics);

#ifdef __cplusplus
}
#endif

