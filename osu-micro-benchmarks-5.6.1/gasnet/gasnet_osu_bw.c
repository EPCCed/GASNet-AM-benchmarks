#define BENCHMARK "OSU MPI%s Bandwidth Test"
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
#include <mpi.h>
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
    int myid, numprocs, i, j;
    int size;
    double t_start = 0.0, t_end = 0.0, t = 0.0;
    int window_size = 64;
    int po_ret = 0;
    options.bench = PT2PT;
    options.subtype = LAT;

    set_header(HEADER);
    set_benchmark_name("osu_bw");
    
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

    if(myid == 0) fprintf(stdout, "%-*s%*s\n", 10, "# Size", FIELD_WIDTH, "Bandwidth (MB/s)");
    
    /* Bandwidth test */
    options.min_message_size = 1;
    for(size = options.min_message_size; size <= options.max_message_size; size *= 2) 
    {
        set_buffer_pt2pt(s_buf, myid, options.accel, 'a', size);
        set_buffer_pt2pt(r_buf, myid, options.accel, 'b', size);


        if(size > LARGE_MESSAGE_SIZE) {
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

                for(j = 0; j < window_size; j++) 
                {
                    /* old MPI command */
                    //MPI_CHECK(MPI_Isend(s_buf, size, MPI_CHAR, 1, 100, MPI_COMM_WORLD,request + j));
                    
                    /* new GASNet implementation */
                    gasnet_send(1, s_buf, size);
                }
                
                /* old MPI commands */
                // MPI_CHECK(MPI_Waitall(window_size, request, reqstat));
                // MPI_CHECK(MPI_Recv(r_buf, 4, MPI_CHAR, 1, 101, MPI_COMM_WORLD,&reqstat[0]));
                
                /* new GASNet implementation */
                GASNET_BLOCKUNTIL(received_msg_count == 1);
                received_msg_count = 0;
            }

            t_end = my_time();
            t = t_end - t_start;
        }

        else if(myid == 1) 
        {
            for(i = 0; i < options.iterations + options.skip; i++) 
            {              
                /* old MPI command */  
                // for(j = 0; j < window_size; j++) 
                // {
                //      MPI_CHECK(MPI_Irecv(r_buf, size, MPI_CHAR, 0, 100, MPI_COMM_WORLD,request + j));
                    
                // }
                
                /* new GASNet implementation */
                GASNET_BLOCKUNTIL(received_msg_count == window_size);
                received_msg_count = 0;

                /* old MPI command */ 
                // MPI_CHECK(MPI_Waitall(window_size, request, reqstat));
                // MPI_CHECK(MPI_Send(s_buf, 4, MPI_CHAR, 0, 101, MPI_COMM_WORLD));
                
                /* new GASNet implementation */
                gasnet_send(0, s_buf, 4);
            }
        }

        if(myid == 0) {
            double tmp = size / 1e6 * options.iterations * window_size;

            fprintf(stdout, "%-*d%*.*f\n", 10, size, FIELD_WIDTH,
                    FLOAT_PRECISION, tmp / t);
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
