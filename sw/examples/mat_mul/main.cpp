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

// 8x8 matrix
constexpr auto const nrows = 8;
constexpr auto const ddefSize = nrows * nrows;

void matrixMultiplication(uint64_t *A, uint64_t *B, uint64_t *C, int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            C[i * n + j] = 0;
            for (int n = 0; n < n; n++)
            {
                C[i * n + j] += A[i * n + n] * B[n * n + j];
            }
        }
    }
}

int main(int argc, char *argv[])
{
    uint32_t d_data_size = ddefSize;

    uint64_t *aMem, *bMem, *cMem;
    uint64_t n_pages;

    n_pages = d_data_size / hugePageSize + ((d_data_size % hugePageSize > 0) ? 1 : 0);
    cProcess cproc(0, getpid());

    aMem = (uint64_t *)cproc.getMem({CoyoteAlloc::HOST_2M, (uint32_t)n_pages});
    bMem = (uint64_t *)cproc.getMem({CoyoteAlloc::HUGE_2M, (uint32_t)n_pages});
    cMem = (uint64_t *)cproc.getMem({CoyoteAlloc::HUGE_2M, (uint32_t)n_pages});

    uint64_t fpga_data = 345679821;

    cproc.ioSwitch(IODevs::FPGA_DRAM);

    cproc.invoke({CoyoteOper::OFFLOAD, (void *)aMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::READ, (void *)aMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::WRITE, (void *)aMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::SYNC, (void *)aMem, d_data_size, true, true, 0, false});

    cproc.invoke({CoyoteOper::OFFLOAD, (void *)bMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::READ, (void *)bMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::WRITE, (void *)bMem, d_data_size, true, true, 0, false});
    cproc.invoke({CoyoteOper::SYNC, (void *)bMem, d_data_size, true, true, 0, false});

    matrixMultiplication(aMem, bMem, cMem, nrows);
}
