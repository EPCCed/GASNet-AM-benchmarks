# GASNet-AM-benchmarks

All relvant code and benchmarks for paper submitted to "Parallel Applications Workshop - Alternatives to MPI+X 2019".

# Hints to compile the binaries

The makefiles require a number of variables set:

* GASNET_INSTALL_DIR: Path to the GASNet installation. They assume this folder contains at least the folders 'include' and 'lib', with the GASNet headers and libraries.
* CONDUIT: The used GASNet conduit in lower-case letters (e.g. mpi, smp, aries, ibv...)
* MPICC:   The MPI-compiler-wrapper. It is important that this is the same one which is used when compiling gasnet.

## osu-microbenchmarks

Here the original automake-based configuration is replaced by new makefiles. Only the relevant code for latency and bandwidth p2p measurments is contained. The latency-benchmark for GASNet has two versions (blocking and non-blocking) to ensure comparability with MPI-versions, which are implemented by synchronous MPI calls. An additional osu_util_gasnet.h/.c was added to the util-folder, which just includes the functionalities neede for the GASNet implementation.

## stencil

The the make file options are the same as in the original benchmark. For the plots in the paper the default settings are used (radius=2,compact,double precision,no restrict,star).

## graph500

For GASNet integration the only changes are made in the aml-layer in the corresponding folder. In the header aml.h, the only changes are some additional barriers in the reduce-defines (to ensure better interoperability between GASNet and MPI). There is also a small test-program included to test the basic functionality of the aml layer.

The makefile in the src-folder requires a environment variable TARGET (= 'mpi' || 'gasnet'). Then the binaries are compiled into seperate folders.

# GASNet configurations

All GASNet-configurations were compiled with the compile-flag '-fPIC'.
During all runs the variable GASNET_MPI_THREAD=SERIALIZED was set.

## ARCHER 

ARCHER is a cluster based on the Aries-interconnect. More details about hardware can be found [here](https://www.archer.ac.uk/about-archer/hardware/). For the tests, the aries-conduit was used without any special compile options:

    ../cross-configure-cray-aries-alps --prefix=$GASNET_INSTALL_DIR CC="cc -fPIC" CXX="CC -fPIC" MPICC="cc -fPIC"

## CIRRUS

CIRRUS is a cluster based on a Infiniband-interconnect. More details about hardware can be found [here](http://www.cirrus.ac.uk/about/hardware.html). For the test, the ibv-conduit was used, with some special compile-flags (to workaround some runtime warnings/errors):

    ../configure --prefix=$GASNET_INSTALL_DIR --disable-ibv-odp --with-ibv-physmem-max=4G --disable-ibv-xrc --enable-pshm-sysv --disable-pshm-posix  CC="gcc -fPIC" CXX="g++ -fPIC" MPICC="mpicc -fPIC"
    

