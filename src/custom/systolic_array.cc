/**
 * Copyright (c) 2025
 * Authors: Christoffer Åleskog (caleskog)
 */
#include <cstdlib>

#include "systolic_array.hh"

// Coudn't include it in the systolic_array_base.hh due to
// it inducing circular dependencies
#ifndef DEVELOP
#include "arch/arm/system.hh"
#include "arch/generic/memhelpers.hh"
#include "base/logging.hh"
#include "cpu/exec_context.hh"
#include "cpu/thread_context.hh"
#include "mem/se_translating_port_proxy.hh"
#include "sim/insttracer.hh"
#include "sim/mem_state.hh"
#include "sim/process.hh"
#endif

#include <cstdio>
#include <cstring>
#include <string>

// #define SMM_PRINT 1

#include "defs.h"
#include "utils.hh"

#define PRINT_ARGUMENTS_LOAD(_c, val, idx, tid)                               \
    if (!use_buffer)                                                          \
    {                                                                         \
        DPRINTF(SA, "[load]: val (%ld): %f (%x)\n", val, _c.f, _c.u);         \
        DPRINTF(SA, "[load]: idx: %ld (%x)\n", idx, idx);                     \
        DPRINTF(SA, "[load]: tid: %ld (%x)\n", tid, tid);                     \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        for (int i = 0; i < buffer_size; i++)                                 \
        {                                                                     \
            DPRINTF(SA, "[load]: val %ld @ %d: %f\n", val, i, _c.p[i]);       \
        }                                                                     \
        DPRINTF(SA, "[load]: idx: %ld (%x)\n", idx, idx);                     \
        DPRINTF(SA, "[load]: tid: %ld (%x)\n", tid, tid);                     \
    }
#define PRINT_ARGUMENTS_QUEUE(_c, val, col, tid)                              \
    if (!use_buffer)                                                          \
    {                                                                         \
        DPRINTF(SA, "[queue]: val (%ld): %f (%x)\n", val, _c.f, _c.u);        \
        DPRINTF(SA, "[queue]: col: %ld (%x)\n", col, col);                    \
        DPRINTF(SA, "[queue]: tid: %ld (%x)\n", tid, tid);                    \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        for (int i = 0; i < buffer_size; i++)                                 \
        {                                                                     \
            DPRINTF(SA, "[queue]: val %ld @ %d: %f\n", val, i, _c.p[i]);      \
        }                                                                     \
        DPRINTF(SA, "[queue]: col: %ld (%x)\n", col, col);                    \
        DPRINTF(SA, "[queue]: tid: %ld (%x)\n", tid, tid);                    \
    }
#define PRINT_ARGUMENTS_PROCESS(_c, val, tid)                                 \
    if (!use_buffer)                                                          \
    {                                                                         \
        DPRINTF(SA, "[process]: val (%ld): %f (%x)\n", val, _c.f, _c.u);      \
        DPRINTF(SA, "[process]: tid: %ld (%x)\n", tid, tid);                  \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        for (int i = 0; i < buffer_size; i++)                                 \
        {                                                                     \
            DPRINTF(SA, "[process]: val %ld @ %d: %f\n", val, i, _c.p[i]);    \
        }                                                                     \
        DPRINTF(SA, "[process]: tid: %ld (%x)\n", tid, tid);                  \
    }

// Using more than one input values
#define PRINT_ARGUMENTS_LOAD2(_tc, _vsrc, _buf, _size, idx, _tid)             \
    {                                                                         \
        DPRINTF(SA, "[load2]: input: %ld (%p)\n", _vsrc, _vsrc);              \
        SETranslatingPortProxy proxy(_tc);                                    \
        proxy.readBlob(_vsrc, (double*)_buf, _size * sizeof(double));         \
        for (int i = 0; i < _size; i++)                                       \
        {                                                                     \
            DPRINTF(SA, "[load2]:   @%d: %f (%x)\n", i, _buf[i], _buf[i]);    \
        }                                                                     \
        DPRINTF(SA, "[load2]: idx: %ld (%x)\n", idx, idx);                    \
        DPRINTF(SA, "[load2]: tid: %ld (%x)\n", tid, tid);                    \
    }
#define PRINT_ARGUMENTS_QUEUE2(_tc, _vsrc, _vdst, _buf, _size, col)           \
    {                                                                         \
        DPRINTF(SA, "[queue2]: input: %ld (%p)\n", _vsrc, _vsrc);             \
        SETranslatingPortProxy proxy(_tc);                                    \
        proxy.readBlob(_vsrc, (double*)_buf, _size * sizeof(double));         \
        for (int i = 0; i < _size; i++)                                       \
        {                                                                     \
            DPRINTF(SA, "[queue2]:   @%d: %f (%x)\n", i, _buf[i], _buf[i]);   \
        }                                                                     \
        DPRINTF(SA, "[queue2]: col: %ld (%x)\n", col, col);                   \
    }
// DPRINTF(SA, "[queue2]: output: %ld (%p)\n", _vdst, _vdst);                 \
    // proxy.readBlob(_vdst, (double *)_buf, _size * sizeof(double));         \
    // for (int i = 0; i < _size; i++) {                                      \
    //   DPRINTF(SA, "[queue2]:   @%d: %f (%x)\n", i, _buf[i], _buf[i]);      \
    // }                                                                      \
  // }

#define PRINT_ARGUMENTS_PROCESS2(_tc, _vsrc, _vdst, _buf, _size)              \
    {                                                                         \
        DPRINTF(SA, "[process2]: input: %ld (%p)\n", _vsrc, _vsrc);           \
        SETranslatingPortProxy proxy(_tc);                                    \
        proxy.readBlob(_vsrc, (double*)_buf, _size * sizeof(double));         \
        for (int i = 0; i < _size; i++)                                       \
        {                                                                     \
            DPRINTF(SA, "[process2]:   @%d: %f (%x)\n", i, _buf[i], _buf[i]); \
        }                                                                     \
    }

//   DPRINTF(SA, "[process2]: output: %ld (%p)\n", _vdst, _vdst);             \
  //   proxy.readBlob(_vdst, (double *)_buf, _size * sizeof(double));         \
  //   for (int i = 0; i < _size; i++) {                                      \
  //     DPRINTF(SA, "[process2]:   @%d: %f (%x)\n", i, _buf[i], _buf[i]);    \
  //   }                                                                      \
  // }

namespace gem5
{

    // Constructor.
    SystolicArray::SystolicArray(const Params& p) : SimObject(p)
    {
        std::string warn_str = "SA core instantiated.";
        std::string macro_str;

#ifdef SMM_PRINT
        macro_str += "SMM_PRINT, ";
#endif
#ifdef KERNEL_DIM
        macro_str += myutils::string_format("KERNEL_DIM=(%d;%d;%d), ",
                                            KERNEL_DIM, MAX_COLS, MAX_ROWS);
#else
        macro_str += myutils::string_format("SA_SIZE=(%d), ", SA_SIZE);
#endif

        // Remove last ',' from string
        if (macro_str.empty())
        {
            warn(warn_str);
        }
        else
        {
            macro_str.pop_back(); // remove ','
            macro_str.pop_back(); // remove ' '
            warn(warn_str + " ( " + macro_str + " )");
        }

        asm_count = 0;

        for (int cores = 0; cores < p.cores; cores++)
        {
            tiles.push_back(new SATile());
        }

        // For the shared buffer allocated from the SimObject
        // read/write with:
        //   proxy.{read,write}Blob(buffer_addr, buffer, alloc_size)
        alloc_size           = ArmSystem::PageBytes;
        internal_buffer_size = alloc_size / sizeof(f64);
        buffer               = new f64[internal_buffer_size];
        buffer_size          = 0;
    }

    // Destructor
    SystolicArray::~SystolicArray()
    {
        for (auto& tile : tiles)
        {
            delete tile;
        }
        delete[] buffer;
    }

    bool SystolicArray::load(u64 tid, u64 col, Addr input)
    {
        DPRINTF(SA, "calling sa->load(tid:%ld, col:%ld, input:%ld)\n", tid,
                col, input);
        check_buffer_usage();
        const int TID                 = tid;
        const int SIZE                = buffer_size;
        const int DEFUALT_SIZE        = 255;
        double    inbuf[DEFUALT_SIZE] = {0};

        PRINT_ARGUMENTS_LOAD2(tc, input, inbuf, SIZE, col, tid);

        if (!getVals(tc, input, inbuf, SIZE))
        {
            panic("[load] Cannot get values from vsrc %p address!", input);
        }

        // Place value in memory
        int col_idx;
        int row_idx;
        int mask = 1;
        for (size_t i = 0; i < SIZE; i++)
        {
            row_idx = MAX_ROWS - 1 - (col + i);
            col_idx = col + i;
            // off-diagonal of the waiting memory
            mem2d(tiles[TID]->inTopWaitingMemory, KERNEL_DIM, row_idx,
                  col_idx) = inbuf[i];
            mask &= (uint64_t)inbuf[i];
            if (mask != 0)
            {
                tiles[TID]->non_zero_tile = true;
            }
        }
        /*
    c64 elm;
    int mask = 1;
    for (size_t i = 0; i < SIZE; i++) {
      tiles[tid]->weights[col + i] = inbuf[i];
      elm.f = inbuf[i];
      mask &= elm.u;
    }
    if (mask != 0) {
      tiles[tid]->non_zero_tile = true;
    } */

        DPRINTF(SA, "[load] res: %ld (%x)\n", tiles[tid]->non_zero_tile,
                tiles[tid]->non_zero_tile);

        return tiles[tid]->non_zero_tile;
    }

    void SystolicArray::queue2(ExecContext* xc, Addr vdst, Addr vsrc, u64 col)
    {
        DPRINTF(SA, "calling sa->queue2(output:%p, input:%p, col:%ld)\n", vdst,
                vsrc, col);

        check_buffer_usage();

        const int SIZE              = buffer_size;
        const int DEFUALT_SIZE      = 255;
        double    buf[DEFUALT_SIZE] = {0};

        c64 in;
        in.u = vsrc;
        c64 out;
        out.u = vdst;

        /* Process *proc = tc->getProcessPtr();
  if (!proc->pTable->translate(vsrc, in.u)) {
    panic("[SA] Cannot translate address %p to physical address", vsrc);
  }
  DPRINTF(SA, "[queue2]: translating vaddr %p into paddr %p\n", vsrc, in.p);
  */

        // Did not work, as we need a load/store like instruction:
        // implements initiateAcc and completeAcc. Though,
        // gem5::readMemAtomicLE should not need it, I think. But it crashed
        // due to memRead not being implemented, which I assume is due to the
        // lack of the correct template used, i.e. implements Execute,
        // InitiateAcc, CompleteAcc.
        /* for (int i = 0; i < SIZE; i++) {
    // uint64_t val = getVal<uint64_t>(xc, vsrc, i);
    uint64_t val;
    Fault fault = gem5::readMemAtomicLE(
        xc, nullptr, vsrc + (i * sizeof(uint64_t)), val, Request::UNCACHEABLE);
    if (fault) {
      panic("[SystolicArray::getVal] initiateMemRead on vaddr %p failed!",
            vsrc);
    }
    DPRINTF(SA, "[queue]:   @%ld: %ld\n", i, val);

    buff[i] = buff[i] + 10;

    uint64_t latency = 0;
    uint64_t val = buff[i];
    Fault f3 =
        gem5::writeMemAtomicLE(xc, nullptr, val, vdst + (i * sizeof(val)),
                               Request::UNCACHEABLE, &latency);
    if (f3) {
      panic("[queue2] writeMemTiming on vaddr %p failed!", vdst);
    }
  } */

        PRINT_ARGUMENTS_QUEUE2(tc, vsrc, vdst, buf, SIZE, col);

        if (!getVals(tc, vsrc, buf, SIZE))
        {
            panic("[queue2] Cannot get values from vsrc %p address!", vsrc);
        }
        for (int i = 0; i < SIZE; i++)
        {
            buf[i] = buf[i] + 10;
        }
        if (!setVals(tc, vdst, buf, SIZE))
        {
            panic("[queue2] Cannot set values in vsrc %p address!", vsrc);
        }
    }

    Addr SystolicArray::queue(Addr vdst, u64 col, Addr vsrc)
    {
        DPRINTF(SA, "calling sa->queue(output:%ld, col:%ld, input:%ld)\n",
                vdst, col, vsrc);
        check_buffer_usage();
        const int TID                  = 0;
        const int SIZE                 = buffer_size;
        const int DEFUALT_SIZE         = 255;
        double    inbuf[DEFUALT_SIZE]  = {0};
        double    outbuf[DEFUALT_SIZE] = {0};

        PRINT_ARGUMENTS_QUEUE2(tc, vsrc, vdst, inbuf, SIZE, col);

        if (!getVals(tc, vsrc, inbuf, SIZE))
        {
            panic("[queue2] Cannot get values from vsrc %p address!", vsrc);
        }
        // asm_count++;
        // c64 converter;
        // converter.u = val;
        //
        // check_buffer_usage();
        // if (use_buffer) {
        //   converter.p = retriveVals();
        // }
        // PRINT_ARGUMENTS_QUEUE(converter, val, col, tid);

        // Place value in memory
        int col_idx;
        for (size_t i = 0; i < SIZE; i++)
        {
            // inform("[queue] val[%d]: %lf", i, converter.p[i]);
            col_idx = col + i;
            // off-diagonal of the waiting memory
            mem2d(tiles[TID]->inWaitingMemory, KERNEL_DIM, col_idx,
                  KERNEL_DIM - col_idx - 1) = inbuf[i];
        }

        // Return the output
        int row_idx;
        for (size_t i = 0; i < SIZE; i++)
        {
            row_idx   = KERNEL_DIM - 1 - (col + i);
            col_idx   = col + i;
            outbuf[i] = mem2d(tiles[TID]->outWaitingMemory, KERNEL_DIM,
                              row_idx, col_idx);
        }

        for (size_t i = 0; i < SIZE; i++)
        {
            DPRINTF(SA, "[queue] result[%d]: %lf\n", i, outbuf[i]);
        }

        if (!setVals(tc, vdst, outbuf, SIZE))
        {
            panic("[queue2] Cannot set values in vsrc %p address!", vsrc);
        }

        return vdst;
    }

    Addr SystolicArray::process(Addr vdst, Addr vsrc)
    {
        DPRINTF(SA, "calling sa->process(output:%ld, input:%ld)\n", vdst,
                vsrc);
        // asm_count++;

        check_buffer_usage();
        const int TID                  = 0;
        const int SIZE                 = buffer_size;
        const int DEFUALT_SIZE         = 255;
        double    inbuf[DEFUALT_SIZE]  = {0};
        double    outbuf[DEFUALT_SIZE] = {0};

        PRINT_ARGUMENTS_PROCESS2(tc, vsrc, vdst, inbuf, SIZE);

        if (!getVals(tc, vsrc, inbuf, SIZE))
        {
            panic("[queue2] Cannot get values from vsrc %p address!", vsrc);
        }

        tiles[TID]->non_zero_tile = false;
        tiles[TID]->nrtowavefronts =
            (tiles[TID]->nrtowavefronts + 1) % KERNEL_DIM;
        if (tiles[TID]->nrtowavefronts == 0)
        {
            // Set wavefront such that when the first value is computed it's
            // taken first.
            tiles[TID]->wavefront.push_back(0);
        }
        // printf("nr of wavefronts: %zu\n", tiles[TID]->wavefront.size());

        //////////////////////////////////////////////////////////
        ///////////////////// GET INPUT //////////////////////////
        int col = MAX_COLS - SIZE;
        int row_index;
        int col_index;
        for (size_t i = 0; i < SIZE; i++)
        {
            // warn("[process] val[%d]: %lf", i, converter.p[i]);
            row_index        = col + i;
            col_index        = KERNEL_DIM - row_index - 1;
            mem2d(tiles[TID]->inWaitingMemory, KERNEL_DIM, row_index,
                  col_index) = inbuf[i];
        }
        //////////////////////////////////////////////////////////

#ifdef SMM_PRINT
        myutils::sa::PrintTile smm_print(tiles[TID], 2, 2);
#endif

        // Set the top waiting memory
        for (int i = 0; i < KERNEL_DIM; i++)
        {
            mem2d(tiles[TID]->weights, KERNEL_DIM, 0, i) = mem2d(
                tiles[TID]->inTopWaitingMemory, KERNEL_DIM, KERNEL_DIM - 1, i);
        }
#ifdef SMM_PRINT
        smm_print.print_top();
#endif
        // Shift the top waiting memory down
        for (int i = 0; i < KERNEL_DIM; i++)
        {
            for (int j = KERNEL_DIM - 1; j > 0; j--)
            { // TODO: shift only the right-hand triangle
                mem2d(tiles[TID]->inTopWaitingMemory, KERNEL_DIM, j, i) =
                    mem2d(tiles[TID]->inTopWaitingMemory, KERNEL_DIM, j - 1,
                          i);
            }
            mem2d(tiles[TID]->inTopWaitingMemory, KERNEL_DIM, 0, i) = 0;
        }

        // Shift the waiting memory to the right for skewing
        for (int i = 0; i < KERNEL_DIM; i++)
        {
            mem2d(tiles[TID]->inputMemory, KERNEL_DIM, i, 0) = mem2d(
                tiles[TID]->inWaitingMemory, KERNEL_DIM, i, KERNEL_DIM - 1);
            for (int j = KERNEL_DIM - 1; j > 0; j--)
            { // TODO: shift only the right-hand triangle
                mem2d(tiles[TID]->inWaitingMemory, KERNEL_DIM, i, j) =
                    mem2d(tiles[TID]->inWaitingMemory, KERNEL_DIM, i, j - 1);
            }
        }

        // Multiply the input to the weight and accumulate to the output
        for (int i = (KERNEL_DIM * KERNEL_DIM) - 1; i >= 0; i--)
        {
            /* tiles[TID]->outputMemory[i + KERNEL_DIM] =
        (tiles[TID]->inputMemory[i] * tiles[TID]->weights[i]) +
        tiles[TID]->outputMemory[i]; */
            tiles[TID]->outputMemory[i] =
                (tiles[TID]->inputMemory[i] * tiles[TID]->weights[i]) +
                tiles[TID]->outputMemory[i];
        }

#ifdef SMM_PRINT
        smm_print.print_fifo_tile();
#endif

        // Shift the top input memory down
        for (int i = 0; i < KERNEL_DIM; i++)
        {
            for (int j = KERNEL_DIM - 1; j > 0; j--)
            {
                tiles[TID]->weights[(j * KERNEL_DIM) + i] =
                    tiles[TID]->weights[((j - 1) * KERNEL_DIM) + i];
            }
        }
        // Shift the input memory to the right
        for (int i = 0; i < KERNEL_DIM; i++)
        {
            for (int j = KERNEL_DIM - 1; j > 0; j--)
            {
                tiles[TID]->inputMemory[(i * KERNEL_DIM) + j] =
                    tiles[TID]->inputMemory[(i * KERNEL_DIM) + j - 1];
            }
        }

        //////////////////////////////////////////////////////////
        /////////////////// RETURN OUTPUT ////////////////////////
        /**
         * Must do this before shifting the outWaitingMemory array.
         * That way, we get the result corresponding to the preceding queue
         *calls in this process call too.
         **/
        // Return the output
        int row_idx = KERNEL_DIM - 1;
        int col_idx = 0;
        for (size_t i = 0; i < SIZE; i++)
        {
            row_idx   = SIZE - 1 - i;
            col_idx   = KERNEL_DIM - SIZE + i;
            outbuf[i] = mem2d(tiles[TID]->outWaitingMemory, KERNEL_DIM,
                              row_idx, col_idx);
        }

        for (size_t i = 0; i < SIZE; i++)
        {
            DPRINTF(SA, "[process] result[%d]: %lf\n", i, outbuf[i]);
        }

        if (!setVals(tc, vdst, outbuf, SIZE))
        {
            panic("[queue2] Cannot set values in vsrc %p address!", vsrc);
        }
        //////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////

#ifdef SMM_PRINT
        smm_print.print_out();
#endif

        // Shift the outWaitingMemory because of the skew in the output
        for (int j = 0; j < KERNEL_DIM; j++)
        {
            for (int i = KERNEL_DIM - 1; i > 0; i--)
            { // TODO: shift only the right-hand triangle
                mem2d(tiles[TID]->outWaitingMemory, KERNEL_DIM, i, j) =
                    mem2d(tiles[TID]->outWaitingMemory, KERNEL_DIM, i - 1, j);
            }
            mem2d(tiles[TID]->outWaitingMemory, KERNEL_DIM, 0, j) =
                mem2d(tiles[TID]->outputMemory, KERNEL_DIM, KERNEL_DIM, j);
        }

        // Copy the wavefronts to the correct location in the waiting output
        // array
        auto last       = tiles[TID]->wavefront.end();
        int  nr_removes = 0;
        for (auto wavefront = tiles[TID]->wavefront.begin();
             wavefront != tiles[TID]->wavefront.end(); ++wavefront)
        {
            if (*wavefront >= 0)
            {
                for (int wave = 0; wave < KERNEL_DIM; wave++)
                {
                    int y = *wavefront - wave;
                    // printf("wavefront: %d\n(0,%d)[wave] =
                    // (%d,%d)[y,wave]\n", *wavefront, wave, y, wave);
                    if (y >= 0 && y < KERNEL_DIM)
                    {
                        mem2d(tiles[TID]->outWaitingMemory, KERNEL_DIM, 0,
                              wave) = mem2d(tiles[TID]->outputMemory,
                                            KERNEL_DIM, y, wave);
                        mem2d(tiles[TID]->outputMemory, KERNEL_DIM, y, wave) =
                            0;
                    }
                }
            }
            *wavefront = (*wavefront + 1) % (KERNEL_DIM + KERNEL_DIM - 1);
            if (*wavefront == 0)
            {
                last = wavefront;
                nr_removes++;
            }
        }
        // if (nr_removes > 0) {
        //   tiles[TID]->wavefront.erase(tiles[TID]->wavefront.begin(), last);
        // }

        // std::cout << "[ASM] count: " << asm_count << std::endl;
        return vdst;
    }

    void SystolicArray::setThreadContex(ThreadContext* tc)
    {
        inform("Setting ThreadContext for SystolicArray");
        this->tc = tc;
    }

    // SA initialization.
    void SystolicArray::init()
    {
        // DPRINTF(SA, "setting output buffer address to %#x\n", buffer_addr);
        // }
        // postInit()

        // HACK: Require the user to set the last element in the buffer
        // (buffer[internal_buffer_size-1]) to not zero to indicate usage of
        // the buffer.
        allocateOutputBuffer();
    }

    void SystolicArray::startup() {}

    // Serialize SA.
    void SystolicArray::serialize(CheckpointOut& cp) const
    {
        warn("SA serialization not yet implemented.");
    }

    // Unserialize SA.
    void SystolicArray::unserialize(CheckpointIn& cp)
    {
        warn("SA deserialization not yet implemented.");
    }

    // Not used as I now take the input address as the buffer directly.
    void SystolicArray::allocateOutputBuffer()
    {
#ifndef DEVELOP
        Process*                  process   = this->tc->getProcessPtr();
        std::shared_ptr<MemState> mem_state = process->memState;
        size_t real_size = roundUp(alloc_size, ArmSystem::PageBytes);
        // Choose a buffer address in guest VA space (e.g., current 'brk')
        Addr addr = mem_state->getBrkPoint();
        // Update brk and create VMA
        mem_state->updateBrkRegion(addr, addr + real_size);
        // Allocate physical memory att the address
        process->allocateMem(addr, real_size, /*cobbler=*/false);
        // Setting the buffer address
        buffer_addr = addr;

        inform("Allocating output buffer of size %ld to address %#x",
               alloc_size, buffer_addr);

        // Zero out the buffer
        inform("Setting data of allocated buffer %#x of size %ld to zeros",
               buffer_addr, alloc_size);
        SETranslatingPortProxy proxy(tc);
        f64                    tmp_buf[ArmSystem::PageBytes] = {0};
        proxy.writeBlob(buffer_addr, tmp_buf, real_size);
#endif
    }

    void SystolicArray::check_buffer_usage()
    {
        if (!checked_buffer_usage)
        {
            checked_buffer_usage = true;
            f64* vals            = retriveVals();
            inform("Checking last element in buffer (using the buffer): %x",
                   vals[internal_buffer_size - 1]);
            if ((uint64_t)vals[internal_buffer_size - 1] != 0)
            {
                use_buffer  = true;
                buffer_size = vals[internal_buffer_size - 2];
                inform("Using buffer_addr in custom instructions with "
                       "buffer_size as %d",
                       buffer_size);
            }
        }
    }

    f64* SystolicArray::retriveVals()
    {
#ifndef DEVELOP
        SETranslatingPortProxy proxy(tc);
        proxy.readBlob(buffer_addr, (uint8_t*)buffer, alloc_size);
        DPRINTF(SA, "Reading %ld 64-bit values from virtual address: %#x\n",
                alloc_size, buffer_addr);
        // Print out the first few values for debugging
        for (size_t i = 0; i < buffer_size; i++)
        {
            DPRINTF(SA, "read %f @ %d\n", buffer[i], i);
        }
#endif
        return buffer;
    }

    void SystolicArray::setVals()
    {
#ifndef DEVELOP
        SETranslatingPortProxy proxy(tc);
        proxy.writeBlob(buffer_addr, (uint8_t*)buffer, alloc_size);
        DPRINTF(SA, "Writting %ld 64-bit values from virtual address: %#x\n",
                alloc_size, buffer_addr);
        // Print out the first few values for debugging
        for (size_t i = 0; i < buffer_size; i++)
        {
            DPRINTF(SA, "write %f @ %d\n", buffer[i], i);
        }
#endif
    }

    /* void SystolicMatrixMultiplication::postInit() {
    ThreadContext *tc = system->getThreadContext(0);
    this->allocateOutputBuffer(tc);
} */

}; // namespace gem5
