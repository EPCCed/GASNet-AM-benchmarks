#define BENCHMARK "OSU MPI%s Latency Test"
/*
 * Copyright (C) 2002-2019 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University. 
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */
#include <gasnet.h>
#include <osu_util_gasnet.h>

#define BARRIER()                                           \
do {                                                        \
  gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);    \
  gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);      \
} while (0)

const gasnet_handler_t req_handler_id = 250;
gasnet_seginfo_t seginfo_table[2];
char *s_buf, *r_buf;
int received_msg_count = 0;

double my_time()
{
    struct timeval timecheck;

    gettimeofday(&timecheck, NULL);
    return (long)timecheck.tv_sec + (long)timecheck.tv_usec / 1.0e6;
}

void req_handler(gasnet_token_t token, void *buf, size_t size)
{
    memcpy(r_buf, buf, size);
    received_msg_count++;
}

void gasnet_send(gasnet_node_t dest, void *buf, size_t size)
{
    if( size < gasnet_AMMaxMedium() && size > 0 )
        gasnet_AMRequestMedium0(dest, req_handler_id, buf, size);
    else if( size < gasnet_AMMaxLongRequest() && size > 0)
        gasnet_AMRequestLong0(dest, req_handler_id, buf, size, seginfo_table[dest].addr);
    else
    {
        fprintf(stderr, "Message size to big or 0\n");
        gasnet_exit(EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}
        
int main (int argc, char *argv[])
{
    int myid, numprocs, i;
    int size;
    double t_start = 0.0, t_end = 0.0;
    int po_ret = 0;
    options.bench = PT2PT;
    options.subtype = LAT;

    set_header(HEADER);
    set_benchmark_name("osu_latency");

    po_ret = process_options(argc, argv);
    
    /* Init gasnet */
    gasnet_init(&argc, &argv);
    numprocs = gasnet_nodes();
    myid = gasnet_mynode();
    
    int segsize = MIN(gasnet_getMaxLocalSegmentSize(), 2*options.max_message_size);
    int min_heap_offset = 0;
    
    gasnet_handlerentry_t handlers[] = {
        { req_handler_id, (void(*)())req_handler }
    };
    
    gasnet_attach(handlers, sizeof(handlers)/sizeof(gasnet_handlerentry_t), segsize, min_heap_offset);
    
    gasnet_getSegmentInfo(seginfo_table, 2);
    
    /* do some preparation */
    if(numprocs != 2) {
        if(myid == 0) {
            fprintf(stderr, "This test requires exactly two processes\n");
        }

        gasnet_exit(EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }

    if (allocate_memory_pt2pt(&s_buf, &r_buf, myid)) {
        /* Error allocating memory */
        gasnet_exit(EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }

#ifdef BLOCKING
    printf("BLOCKING version\n");
#endif
    
    if(myid == 0) fprintf(stdout, "%-*s%*s\n", 10, "# Size", FIELD_WIDTH, "Latency (us)");

    
    /* Latency test */
    options.min_message_size = 1;
    for(size = options.min_message_size; size <= options.max_message_size; size = (size ? size * 2 : 1)) 
    {
        set_buffer_pt2pt(s_buf, myid, options.accel, 'a', size);
        set_buffer_pt2pt(r_buf, myid, options.accel, 'b', size);

        if(size > LARGE_MESSAGE_SIZE) 
        {
            options.iterations = options.iterations_large;
            options.skip = options.skip_large;
        }

        if(myid == 0) 
        {
            for(i = 0; i < options.iterations + options.skip; i++) 
            {
                if(i == options.skip) 
                {
                    t_start = my_time();
                }

                /* old MPI instructions */
                // MPI_CHECK(MPI_Send(s_buf, size, MPI_CHAR, 1, 1, MPI_COMM_WORLD));
                // MPI_CHECK(MPI_Recv(r_buf, size, MPI_CHAR, 1, 1, MPI_COMM_WORLD, &reqstat));
                
                /* new GASNet implementation. The conditional blocking should replicate the sychronous behaviour of MPI_Send/MPI_Recv */
                gasnet_send(1, s_buf, size);
                
#ifdef BLOCKING
                BARRIER();
#endif
                
                GASNET_BLOCKUNTIL(received_msg_count == 1);
                received_msg_count = 0;
                
#ifdef BLOCKING
                BARRIER();
#endif
            }

            t_end = my_time();
        }

        else if(myid == 1) 
        {
            for(i = 0; i < options.iterations + options.skip; i++) 
            {
                /* old MPI instructions */
                // MPI_CHECK(MPI_Recv(r_buf, size, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &reqstat));
                // MPI_CHECK(MPI_Send(s_buf, size, MPI_CHAR, 0, 1, MPI_COMM_WORLD));
                
                /* new GASNet implementation. The conditional blocking should replicate the sychronous behaviour of MPI_Send/MPI_Recv */
                GASNET_BLOCKUNTIL(received_msg_count == 1);
                received_msg_count = 0;

#ifdef BLOCKING
                BARRIER();
#endif
                
                gasnet_send(0, s_buf, size);
                
#ifdef BLOCKING
                BARRIER();
#endif
            }
        }

        if(myid == 0) 
        {
            double latency = (t_end - t_start) * 1e6 / (2.0 * options.iterations);

            fprintf(stdout, "%-*d%*.*f\n", 10, size, FIELD_WIDTH,
                    FLOAT_PRECISION, latency);
            fflush(stdout);
        }
    }

    free_memory(s_buf, r_buf, myid);

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }

    BARRIER();
    gasnet_exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

