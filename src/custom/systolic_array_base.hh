#ifndef INCLUDE_GEM5_MANAGER_SYSTOLIC_ARRAY_BASE_HH_
#define INCLUDE_GEM5_MANAGER_SYSTOLIC_ARRAY_BASE_HH_

#ifdef DEVELOP

#include <cstdint>
#include <cstring>

#include "systolic_array_utils.hh"

namespace gem5 {

using CheckpointOut = int;
using CheckpointIn = int;

struct Params {
public:
  int cores;
};

class SimObject {
public:
  SimObject(const Params &p){};
  virtual void serialize(CheckpointOut &cp) const {};
  virtual void unserialize(CheckpointIn &cp){};
  virtual void init(){};
  virtual void startup(){};
};

class ThreadContext {
public:
  ThreadContext(){};
};
class ExecContext {
public:
  ExecContext(){};
};

namespace ArmSystem {
static uint64_t PageBytes = 4096;
};

namespace debug {
namespace SA {};
} // namespace debug

class SystolicArray;

using Addr = uint64_t;

class SETranslatingPortProxy {
public:
  SETranslatingPortProxy(gem5::ThreadContext *tc){};
  bool tryReadBlob(Addr vaddr, void *ptr, uint64_t size) {
    std::memcpy(ptr, (void *)vaddr, size);
    return true;
  };
  bool tryWriteBlob(Addr vaddr, void *ptr, uint64_t size) {
    std::memcpy((void *)vaddr, ptr, size);
    return true;
  };
  void readBlob(Addr vaddr, void *ptr, uint64_t size) {
    tryReadBlob(vaddr, ptr, size);
  };
  void writeBlob(Addr vaddr, void *ptr, uint64_t size) {
    tryWriteBlob(vaddr, ptr, size);
  };
};

} // namespace gem5

#define PARAMS(cname) SystolicArray(int cores) : SystolicArray(Params{cores}){};

#ifdef APPLY_TRACE
#define DPRINTF(_, fmt, ...) printf(fmt, __VA_ARGS__);
#else
#define DPRINTF(_, fmt, ...) {};
#endif
#define warn(...) du_pprint(__VA_ARGS__)
#define inform(...) du_pprint(__VA_ARGS__)
#define panic(...)                                                             \
  { du_pprint(__VA_ARGS__) exit(1); }

#else

#include "arch/generic/memhelpers.hh"       // IWYU pragma: export
#include "base/trace.hh"                    // IWYU pragma: export
#include "base/types.hh"                    // IWYU pragma: export
#include "cpu/thread_context.hh"            // IWYU pragma: export
#include "debug/SA.hh"                      // IWYU pragma: export
#include "mem/se_translating_port_proxy.hh" // IWYU pragma: export
#include "params/SystolicArray.hh"          // IWYU pragma: export
#include "sim/sim_object.hh"                // IWYU pragma: export

#endif

#endif // INCLUDE_GEM5-MANAGER_SYSTOLIC_ARRAY_BASE_HH_
