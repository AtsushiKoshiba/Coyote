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

#include "cProcess.hpp"

using namespace fpga;

constexpr auto const ddefSize = 64 * 1024;
constexpr auto const odefSize = 64;

int main(int argc, char *argv[])
{
    uint32_t d_data_size = ddefSize;
    uint32_t o_data_size = odefSize;

    uint64_t *dMem, *oMem;
    uint64_t n_input_pages, n_output_pages;

    n_input_pages = d_data_size / hugePageSize + ((d_data_size % hugePageSize > 0) ? 1 : 0);
    n_output_pages = o_data_size / hugePageSize + ((o_data_size % hugePageSize > 0) ? 1 : 0);

    cProcess cproc(0, getpid());

    uint64_t fpga_data = 10010;

    dMem = (uint64_t *)cproc.getMem({CoyoteAlloc::HOST_2M, (uint32_t)n_input_pages});
    oMem = (uint64_t *)cproc.getMem({CoyoteAlloc::HOST_2M, (uint32_t)n_output_pages});

    memcpy(oMem, &fpga_data, 8);

    std::cout << "Input data mapped at: " << dMem << std::endl;
    std::cout << "Output data mapped at: " << oMem << std::endl;

    std::cout << "dMem:" << *((uint64_t *)dMem) << std::endl;
    std::cout << "oMem before:" << *((uint64_t *)oMem) << std::endl;

    cproc.ioSwDbg();
    cproc.ioSwitch(IODevs::HOST_MEM);
    cproc.ioSwDbg();
    cproc.invoke({CoyoteOper::TRANSFER, (void *)oMem, o_data_size});
    std::cout << "In host mem: oMem after:" << *((uint64_t *)oMem) << std::endl;

    cproc.ioSwDbg();
    cproc.ioSwitch(IODevs::FPGA_DRAM);
    cproc.ioSwDbg();
    cproc.invoke({CoyoteOper::READ, (void *)oMem, o_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::WRITE, (void *)oMem, o_data_size, true, true, 0, false});
    std::cout << "In FPGA mem: oMem after:" << *((uint64_t *)oMem) << std::endl;

    cproc.ioSwDbg();
    cproc.ioSwitch(IODevs::HOST_MEM);
    cproc.ioSwDbg();
    cproc.invoke({CoyoteOper::TRANSFER, (void *)oMem, o_data_size});
    std::cout << "In host mem: oMem after:" << *((uint64_t *)oMem) << std::endl;
}
