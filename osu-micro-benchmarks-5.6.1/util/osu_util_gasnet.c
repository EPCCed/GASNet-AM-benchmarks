/*
 * Copyright (C) 2002-2019 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level directory.
 */

#include "osu_util_gasnet.h"


int process_one_sided_options (int opt, char *arg)
{
    switch(opt) {
        case 'w':
#if MPI_VERSION >= 3
            if (0 == strcasecmp(arg, "create")) {
                options.win = WIN_CREATE;
            } else if (0 == strcasecmp(arg, "allocate")) {
                options.win = WIN_ALLOCATE;
            } else if (0 == strcasecmp(arg, "dynamic")) {
                options.win = WIN_DYNAMIC;
            } else
#endif
            {
                return PO_BAD_USAGE;
            }
            break;
        case 's':
            if (0 == strcasecmp(arg, "pscw")) {
                options.sync = PSCW;
            } else if (0 == strcasecmp(arg, "fence")) {
                options.sync = FENCE;
            } else if (options.synctype== ALL_SYNC) {
                if (0 == strcasecmp(arg, "lock")) {
                    options.sync = LOCK;
                }
#if MPI_VERSION >= 3
                else if (0 == strcasecmp(arg, "flush")) {
                    options.sync = FLUSH;
                } else if (0 == strcasecmp(arg, "flush_local")) {
                    options.sync = FLUSH_LOCAL;
                } else if (0 == strcasecmp(arg, "lock_all")) {
                    options.sync = LOCK_ALL;
                }
#endif
                else {
                    return PO_BAD_USAGE;
                }
            } else {
                return PO_BAD_USAGE;
            }
            break;
        default:
            return PO_BAD_USAGE;
    }

    return PO_OKAY;
}

int allocate_memory_pt2pt (char ** sbuf, char ** rbuf, int rank)
{
    unsigned long align_size = sysconf(_SC_PAGESIZE);

    switch (rank) {
        case 0:
            if ('D' == options.src) {
//                 if (allocate_device_buffer(sbuf)) {
//                     fprintf(stderr, "Error allocating cuda memory\n");
//                     return 1;
//                 }
// 
//                 if (allocate_device_buffer(rbuf)) {
//                     fprintf(stderr, "Error allocating cuda memory\n");
//                     return 1;
//                 }
            } else if ('M' == options.src) {
//                 if (allocate_managed_buffer(sbuf)) {
//                     fprintf(stderr, "Error allocating cuda unified memory\n");
//                     return 1;
//                 }
// 
//                 if (allocate_managed_buffer(rbuf)) {
//                     fprintf(stderr, "Error allocating cuda unified memory\n");
//                     return 1;
//                 }
            } else {
                if (posix_memalign((void**)sbuf, align_size, options.max_message_size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }

                if (posix_memalign((void**)rbuf, align_size, options.max_message_size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }
            }
            break;
        case 1:
            if ('D' == options.dst) {
//                 if (allocate_device_buffer(sbuf)) {
//                     fprintf(stderr, "Error allocating cuda memory\n");
//                     return 1;
//                 }
// 
//                 if (allocate_device_buffer(rbuf)) {
//                     fprintf(stderr, "Error allocating cuda memory\n");
//                     return 1;
//                 }
            } else if ('M' == options.dst) {
//                 if (allocate_managed_buffer(sbuf)) {
//                     fprintf(stderr, "Error allocating cuda unified memory\n");
//                     return 1;
//                 }
// 
//                 if (allocate_managed_buffer(rbuf)) {
//                     fprintf(stderr, "Error allocating cuda unified memory\n");
//                     return 1;
//                 }
            } else {
                if (posix_memalign((void**)sbuf, align_size, options.max_message_size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }

                if (posix_memalign((void**)rbuf, align_size, options.max_message_size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }
            }
            break;
    }

    return 0;
}

void set_buffer_pt2pt (void * buffer, int rank, enum accel_type type, int data, size_t size)
{
    char buf_type = 'H';

    if (options.bench == MBW_MR) {
        buf_type = (rank < options.pairs) ? options.src : options.dst;
    } else {
        buf_type = (rank == 0) ? options.src : options.dst;
    }

    switch (buf_type) {
        case 'H':
            memset(buffer, data, size);
            break;
        case 'D':
        case 'M':
#ifdef _ENABLE_OPENACC_
            if (type == OPENACC) {
                size_t i;
                char * p = (char *)buffer;
                #pragma acc parallel loop deviceptr(p)
                for (i = 0; i < size; i++) {
                    p[i] = data;
                }
                break;
            } else
#endif
#ifdef _ENABLE_CUDA_
            {
                CUDA_CHECK(cudaMemset(buffer, data, size));
            }
#endif
            break;
    }
}

void set_buffer (void * buffer, enum accel_type type, int data, size_t size)
{
#ifdef _ENABLE_OPENACC_
    size_t i;
    char * p = (char *)buffer;
#endif
    switch (type) {
        case NONE:
            memset(buffer, data, size);
            break;
        case CUDA:
        case MANAGED:
#ifdef _ENABLE_CUDA_
            CUDA_CHECK(cudaMemset(buffer, data, size));
#endif
            break;
        case OPENACC:
#ifdef _ENABLE_OPENACC_
            #pragma acc parallel loop deviceptr(p)
            for (i = 0; i < size; i++) {
                p[i] = data;
            }
#endif
            break;
    }
}

void free_memory (void * sbuf, void * rbuf, int rank)
{
    switch (rank) {
        case 0:
//             if ('D' == options.src || 'M' == options.src) {
//                 free_device_buffer(sbuf);
//                 free_device_buffer(rbuf);
//             } else {
                free(sbuf);
                free(rbuf);
//             }
            break;
        case 1:
//             if ('D' == options.dst || 'M' == options.dst) {
//                 free_device_buffer(sbuf);
//                 free_device_buffer(rbuf);
//             } else {
                free(sbuf);
                free(rbuf);
//             }
            break;
    }
}

int cleanup_accel (void)
{
#ifdef _ENABLE_CUDA_
    CUresult curesult = CUDA_SUCCESS;
#endif

    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case MANAGED:
        case CUDA:
            curesult = cuCtxDestroy(cuContext);

            if (curesult != CUDA_SUCCESS) {
                return 1;
            }
            break;
#endif
#ifdef _ENABLE_OPENACC_
        case OPENACC:
            acc_shutdown(acc_device_nvidia);
            break;
#endif
        default:
            fprintf(stderr, "Invalid accel type, should be cuda or openacc\n");
            return 1;
    }

    return 0;
}
