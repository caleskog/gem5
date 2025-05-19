#ifndef INCLUDE_CUSTOM_MYUTILS_HH_
#define INCLUDE_CUSTOM_MYUTILS_HH_

#include <climits>
#include <cmath>
#include <cstdarg>
#include <memory>

#include "colors.hh"      // IWYU pragma: export
#include "utils_print.hh" // IWYU pragma: export

namespace myutils {

/* Used for adding which macros are defined in the initialization
 * of the SMM SimObject class.
 * From: https://stackoverflow.com/a/26221725/21057906
 */
template <typename... Args>
auto string_format(const ::std::string &format, Args... args) -> ::std::string {
  int size_s = ::std::snprintf(nullptr, 0, format.c_str(), args...) +
               1; // Extra space for '\0'
  if (size_s <= 0) {
    throw ::std::runtime_error("Error during formatting.");
  }
  auto size = static_cast<size_t>(size_s);
  ::std::unique_ptr<char[]> buf(new char[size]);
  ::std::snprintf(buf.get(), size, format.c_str(), args...);
  return ::std::string(buf.get(),
                       buf.get() + size - 1); // We don't want the '\0' inside
}

// Check if the host is little endian
static bool isLittleEndian() {
  int num = 1;
  return (*(char *)&num == 1);
};

} // namespace myutils

#endif // INCLUDE_CUSTOM_MYUTILS_HH_
