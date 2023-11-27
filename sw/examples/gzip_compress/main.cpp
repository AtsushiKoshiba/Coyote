#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <cstring>
#include <boost/program_options.hpp>
#include "gzip/compress.hpp"
#include <gzip/utils.hpp>

#include "cProcess.hpp"

using namespace fpga;

constexpr auto const ddefSize = 64;

void gzipCompress(char *fMem, size_t size)
{
    std::string compressed_data = gzip::compress(fMem, size);
    if (!gzip::is_compressed(compressed_data.data(), compressed_data.size()))
    {
        std::cout << "Data compression failed" << std::endl;
    }
    std::cout << "Data compression complete" << std::endl;
}

int main(int argc, char *argv[])
{
    uint32_t d_data_size = ddefSize;

    uint64_t *aMem;
    uint64_t n_pages;

    n_pages = d_data_size / hugePageSize + ((d_data_size % hugePageSize > 0) ? 1 : 0);
    cProcess cproc(0, getpid());

    aMem = (uint64_t *)cproc.getMem({CoyoteAlloc::HOST_2M, (uint32_t)n_pages});

    uint64_t fpga_data = 345679821;

    cproc.ioSwitch(IODevs::FPGA_DRAM);

    cproc.invoke({CoyoteOper::OFFLOAD, (void *)aMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::READ, (void *)aMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::WRITE, (void *)aMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::SYNC, (void *)aMem, d_data_size, true, true, 0, false});

    gzipCompress((char *)aMem, d_data_size);
}
