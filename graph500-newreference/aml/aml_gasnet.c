#include <stdio.h>
#include <stdlib.h>
#include <gasnet.h>
#include <mpi.h>

#include "aml.h"

#define TRUE 1
#define FALSE 0

#define BARRIER()                                           \
do {                                                        \
  gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);    \
  gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);      \
} while (0)

struct handler_func_ptr_t
{
    void(*func_ptr)(int,void*,int);
};

struct remote_memory_t
{
    void *addr;
    size_t size;
    int avail;
};

/* node information */
gasnet_node_t my_node;
gasnet_node_t nodes;

/* global data structures */
static struct handler_func_ptr_t handler_fptrs[256];
gasnet_seginfo_t *seginfo_table;
struct remote_memory_t *remote_addresses;

/* handler ids */
const int short_handler_id = 200;
const int medlong_handler_id = 201;
const int long_reply_handler_id = 202;

/* handlers */
void short_handler(gasnet_token_t token, int real_id) 
{
    gasnet_node_t src_node;
    gasnet_AMGetMsgSource (token , &src_node);
    
    if( !handler_fptrs[real_id].func_ptr )
    {
        fprintf(stderr, "calling non registered handler with id %d on node %d\n", real_id, my_node);
        exit(1);
    }
    
    handler_fptrs[real_id].func_ptr(src_node, NULL, 0); 
}

void medlong_handler(gasnet_token_t token, void *buf, size_t size, int real_id) 
{    
    gasnet_node_t src_node;
    gasnet_AMGetMsgSource (token , &src_node);
    
    if( !handler_fptrs[real_id].func_ptr )
    {
        fprintf(stderr, "calling non registered handler with id %d on node %d\n", real_id, my_node);
        exit(1);
    }
    
    handler_fptrs[real_id].func_ptr(src_node, buf, size); 
    
    if( size > gasnet_AMMaxLongRequest() )
        gasnet_AMReplyShort0(token, long_reply_handler_id);
}

void long_reply_handler(gasnet_token_t token)
{
    gasnet_node_t src_node;
    gasnet_AMGetMsgSource (token , &src_node);
    remote_addresses[src_node].avail = TRUE;
}

/* init GASNet */
int aml_init(int *argc_ptr,char ***argv_ptr)
{    
    /* init gasnet */
    gasnet_init(argc_ptr, argv_ptr);
    
    /* init node information */
    my_node = gasnet_mynode();
    nodes = gasnet_nodes();
    
    /* init segment and handlers */
    size_t segsize = 1024 * 1024; //gasnet_getMaxLocalSegmentSize();
    size_t min_heap_offset = 0;
    
    gasnet_handlerentry_t handlers[] = {
        { short_handler_id,         (void(*)())short_handler },
        { medlong_handler_id,       (void(*)())medlong_handler },
        { long_reply_handler_id,    (void(*)())long_reply_handler }
    };
    
    gasnet_attach(handlers, sizeof(handlers)/sizeof(gasnet_handlerentry_t), segsize, min_heap_offset);
        
    /* allocate memory */
    seginfo_table    = (gasnet_seginfo_t *)malloc( nodes*sizeof(gasnet_seginfo_t) );
    remote_addresses = (struct remote_memory_t *)malloc( (nodes-1)*sizeof(struct remote_memory_t) );
        
    if( !seginfo_table || !remote_addresses )
    {
        fprintf(stderr, "memory allocation failed\n");
        exit(1);
    }
    
    /* init MPI */
    BARRIER();
    
    int isMPIinit;
    if( MPI_Initialized(&isMPIinit) != MPI_SUCCESS )
    {
        fprintf(stderr, "MPI_Initialized() failed\n");
        exit(1);
    }
    if(!isMPIinit) MPI_Init(argc_ptr, argv_ptr);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    /* get segments and init remote memory chunks for long messages */
    gasnet_getSegmentInfo(seginfo_table, nodes);
    
    for(size_t rank = 0; rank < nodes; ++rank)
    {
        if( rank != my_node )
        {        
            struct remote_memory_t rm;
            size_t my_index = rank < my_node ? my_node-1 : my_node;
            rm.size = seginfo_table[rank].size / (nodes-1);
            rm.addr = (char *)seginfo_table[rank].addr + my_index * rm.size;
            rm.avail = TRUE;
            
            remote_addresses[rank] = rm;
        }
    }
    
    /* init function pointers to NULL */
    for(int i=0; i < 256; ++i)
        handler_fptrs[i].func_ptr = NULL;
    
    BARRIER();
}

int aml_my_pe( void )
{
    return my_node;
}

int aml_n_pes( void )
{
    return nodes;
}

void aml_finalize(void)
{
    /* finalize MPI */    
    int isMPIinit;
    if( MPI_Initialized(&isMPIinit) != MPI_SUCCESS )
    {
        fprintf(stderr, "MPI_Initialized() failed\n");
        exit(1);
    }
    if (!isMPIinit) MPI_Finalize();
    
    /* finalize GASNet */
    BARRIER();
    gasnet_AMPoll();
    
    BARRIER();
    gasnet_exit(0);
    
    if( remote_addresses ) free(remote_addresses);
    if( seginfo_table )    free(seginfo_table);
}

void aml_barrier( void )
{
    gasnet_AMPoll();
    BARRIER();
}

void aml_register_handler(void(*f)(int,void*,int),int n)
{
    MPI_Barrier(MPI_COMM_WORLD);
    BARRIER();
    
    gasnet_AMPoll();
    
    if( n < 256 && n > 0 )
    {
        handler_fptrs[n].func_ptr = f;
        
//         if( handler_fptrs[n].func_ptr != NULL )
//             fprintf(stderr, "WARNING: overwriting handler index %d\n", n);
//         else
//             printf("registered handler with index %d\n", n);
    }    
    else
    {
        fprintf(stderr, "registration failed (index %d not in [0, 255])\n", n);
        exit(1);
    }
    
    BARRIER();
    MPI_Barrier(MPI_COMM_WORLD);
}

void aml_send(void *srcaddr, int n,int length, int node )
{    
    gasnet_AMPoll();
    
#ifdef PRINT_MSG_DATA
    if(length % sizeof(int) == 0)
    {
        printf("send integers [");
        for(int i=0; i<length/sizeof(int); ++i) printf(" %d", *((int *)srcaddr+i));
        printf(" ] to node %d\n", node);
    }
#endif
    
    if( node == my_node )
    {
        handler_fptrs[n].func_ptr(my_node, srcaddr, length);
    }
    else
    {
        if( length == 0 )
        {
            gasnet_AMRequestShort1(node, short_handler_id, n);
        }
        else if( length < gasnet_AMMaxMedium() )
        {
            gasnet_AMRequestMedium1(node, medlong_handler_id, srcaddr, length, n);
        }
//         else if( length < gasnet_AMMaxLongRequest() && length < remote_addresses[node].size )
//         {
//             if( remote_addresses[node].avail == FALSE )
//             {
//                 GASNET_BLOCKUNTIL( remote_addresses[node].avail == TRUE );
//             }
//             
//             gasnet_AMRequestLong1(node, medlong_handler_id, srcaddr, length, remote_addresses[node].addr, n);
//             remote_addresses[node].avail = FALSE;
//         }
        else
        {
            fprintf(stderr, "send failed due to message size (to big or negative)\n");
            aml_finalize();
            exit(1);
        }
    }
}
