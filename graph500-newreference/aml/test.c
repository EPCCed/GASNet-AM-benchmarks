#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "aml.h"


double global_double;

void set_global_double(int src_node, void *buf, int size)
{
    global_double = *(double *)buf;
    printf("global_double on node %d replaced by node %d with value %f\n", aml_my_pe(), src_node, global_double);
}

const int id = 4;

int main(int argc, char **argv)
{
    aml_init(&argc, &argv);
    
    double send_value = aml_my_pe() * 10.0;
    int neighbour = (aml_my_pe() + 1) % aml_n_pes();
    
    aml_register_handler(set_global_double, id);
    
    aml_send(&send_value, id, sizeof(double), neighbour);
    
    aml_barrier();
    
    long long sum_nodes = aml_my_pe();
    long long max_nodes = aml_my_pe();
    long long min_nodes = aml_my_pe();
    
    aml_long_allsum(&sum_nodes);
    aml_long_allmax(&max_nodes);
    aml_long_allmin(&min_nodes);
    
    aml_barrier();
    
    if( aml_my_pe() == 0 )
    {
        printf("sum of all ranks = %lld\n", sum_nodes);
        printf("min of all ranks = %lld\n", min_nodes);
        printf("max of all ranks = %lld\n", max_nodes);
    }
    
    aml_finalize();
}
