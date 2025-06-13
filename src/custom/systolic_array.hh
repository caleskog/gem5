/**
 * Copyright (c) 2025
 * Authors: Christoffer Åleskog (caleskog)
 */

#ifndef INCLUDE_ARM_SYSTOLIC_ARRAY_HH_
#define INCLUDE_ARM_SYSTOLIC_ARRAY_HH_

// #include "sim/sim_object.hh"
#include <cstdint>
#include <cstring>
#include <vector>

#include "systolic_array_base.hh"
#include "systolic_array_utils.hh"

namespace gem5
{

    struct SATile
    {
        SATile()
            : weights(new f64[KERNEL_DIM * KERNEL_DIM]),
              inputMemory(new f64[KERNEL_DIM * KERNEL_DIM]),
              outputMemory(new f64[KERNEL_DIM * (KERNEL_DIM + 1)]),
              inWaitingMemory(new f64[KERNEL_DIM * KERNEL_DIM]),
              inTopWaitingMemory(new f64[KERNEL_DIM * KERNEL_DIM]),
              outWaitingMemory(new f64[KERNEL_DIM * (KERNEL_DIM + 2)]),
              resultVecMemory(new f64[KERNEL_DIM])
        {
            const size_t size = KERNEL_DIM * KERNEL_DIM;

            // Reset internal memory arrays
            memset(weights, 0, size * sizeof(f64));
            memset(inputMemory, 0, size * sizeof(f64));
            memset(inWaitingMemory, 0, size * sizeof(f64));
            memset(inTopWaitingMemory, 0, size * sizeof(f64));
            memset(outWaitingMemory, 0, (size + KERNEL_DIM * 2) * sizeof(f64));

            memset(outputMemory, 0, (size + KERNEL_DIM) * sizeof(f64));

            memset(resultVecMemory, 0, KERNEL_DIM * sizeof(f64));
        }

        f64*             weights;         // was int8_t
        f64*             inputMemory;     // was int8_t
        f64*             outputMemory;    // was int32_t
        f64*             inWaitingMemory; // was int8_t
        f64*             inTopWaitingMemory;
        f64*             outWaitingMemory; // was uint8_t
        f64*             resultVecMemory;
        bool             non_zero_tile  = false;
        int              nrtowavefronts = 0;
        std::vector<int> wavefront;
    };

    class SystolicArray : public SimObject
    {
      private:
        // void readMemoryDouble(ThreadContext *tc, Addr virt_addr, double
        // *buffer,
        //                       size_t count);
        // void writeMemoryDouble(ThreadContext *tc, Addr virt_addr,
        //                        const double *buffer, size_t count);

        f64* retriveVals();
        void setVals();

        // template <class T> T getVal(ExecContext *xc, Addr vaddr, uint64_t
        // index); template <class T> void setVal(ExecContext *xc, Addr vaddr,
        // uint64_t index, T &val);
        template <class T>
        bool getVals(ThreadContext* tc, Addr vaddr, T* buf, uint64_t size);
        template <class T>
        bool setVals(ThreadContext* tc, Addr vaddr, T* buf, uint64_t size);

        void allocateOutputBuffer();
        void check_buffer_usage();

        int  internal_buffer_size;
        int  buffer_size;
        f64* buffer;
        bool checked_buffer_usage = false;
        bool use_buffer           = false;

        Addr alloc_size;
        Addr buffer_addr = 0; // Where we allocate the buffer

        int asm_count;

        std::vector<SATile*> tiles;
        ThreadContext*       tc;

      public:
        PARAMS(SystolicArray);
        SystolicArray(const Params& p);
        ~SystolicArray();
        void init() override;
        void startup() override;

        // Required by SimObject.
        void serialize(CheckpointOut& cp) const override;
        void unserialize(CheckpointIn& cp) override;

        void setThreadContex(ThreadContext* tc);
        /* void postInit();
        void allocateOutputBuffer(ThreadContext *tc); */

        bool load(u64 tid, u64 idx, Addr input);
        Addr queue(Addr output, u64 col, Addr input);
        void queue2(ExecContext* xc, Addr output, Addr input, u64 col);
        Addr process(Addr output, Addr input);

        void print(u64 tid);

#ifdef DEVELOP
        f64* getBufferPtr() { return buffer; };
#endif
    };

    /* template <class T>
    T SystolicArray::getVal(ExecContext *xc, Addr vaddr, uint64_t index) {
      T val;
      Fault fault = gem5::initiateMemRead(xc, nullptr, vaddr + (index *
    sizeof(T)), val, Request::UNCACHEABLE); if (fault) {
        panic("[SystolicArray::getVal] initiateMemRead on vaddr %p failed!",
    vaddr);
      }
      return val;
    };
    template <class T>
    void SystolicArray::setVal(ExecContext *xc, Addr vaddr, uint64_t index, T
    &val){
    }; */
    template <class T>
    bool SystolicArray::getVals(ThreadContext* tc, Addr vaddr, T* buf,
                                uint64_t size)
    {
        SETranslatingPortProxy proxy(tc);
        return proxy.tryReadBlob(vaddr, (double*)buf, size * sizeof(double));
    };
    template <class T>
    bool SystolicArray::setVals(ThreadContext* tc, Addr vaddr, T* buf,
                                uint64_t size)
    {
        SETranslatingPortProxy proxy(tc);
        return proxy.tryWriteBlob(vaddr, (double*)buf, size * sizeof(double));
    };

} // namespace gem5

#endif // INCLUDE_ARM_SYSTOLIC_ARRAY_HH_
