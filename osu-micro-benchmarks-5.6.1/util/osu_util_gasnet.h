/*
 * Copyright (C) 2002-2019 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 * 
 * GASNet note: same as osu_util_mpi.h, but without any MPI related functions
 */

#include "osu_util.h"


int allocate_memory_pt2pt (char **sbuf, char **rbuf, int rank);
void print_header(int rank, int full);
void set_buffer_pt2pt (void * buffer, int rank, enum accel_type type, int data, size_t size);
void free_memory (void *sbuf, void *rbuf, int rank);
int cleanup_accel (void);
