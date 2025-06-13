#ifndef INCLUDE_CUSTOM_UTILS_PRINT_HH_
#define INCLUDE_CUSTOM_UTILS_PRINT_HH_

#include <algorithm>
#include <bitset>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <typeinfo>

#include "colors.hh"

namespace myutils
{

    namespace printing
    {

        /**
         * @brief Constructing a string representation of a binaryof number.
         * @tparam n The number to be converted to binary.
         * @return A string containing the binary representation of the number.
         */
        template <typename T> static std::string toBinary(T n)
        {
            std::bitset<sizeof(T) * CHAR_BIT> b(n);
            std::string                       r = b.to_string();
            // Adding spaces between every 4 bits and double spaces between
            // every 8 bits.
            int offset    = 0;
            int size      = r.length();
            int half_byte = std::max(CHAR_BIT / 2, 4);
            for (int i = half_byte; i < size; i += half_byte)
            {
                r.insert(i + offset, " ");
                offset++;
                if (half_byte != CHAR_BIT && i % CHAR_BIT == 0)
                {
                    r.insert(i + offset, " ");
                    offset++;
                }
            }
            return r;
        };

        /**
         * @brief Printing a binary representation of a number.
         */
        template <typename T> static void printBinary(T n)
        {
            std::cout << toBinary(n) << std::endl;
        };

        /**
         * @brief Printing a binary representation of a number.
         */
        template <typename T> static void printBinary(const char* color, T n)
        {
            std::cout << color << toBinary(n) << RESET << std::endl;
        };

/**
 * @brief Macro for printing indices of a tensor and its corresponding value.
 * The indices' names are reflected.
 * @tparam tensor The tensor to be printed.
 * @tparam args The indices of the tensor.
 */
#define du_reflectTensorAt(tensor, ...)                                       \
    std::cout << "[" << #__VA_ARGS__                                          \
              << "] = " << myutils::printing::toBinary(tensor[__VA_ARGS__])   \
              << " (" << tensor[__VA_ARGS__] << ")" << std::endl;

/**
 * @brief Macro for printing Tensor3D and reflecting their values.
 *
 * Example:
 *      int i = 0;
 *      int j = 1;
 *      int k = 2;
 *      reflectTensorAt(tensor, i, j, k); // where tensor->at(i, j, k) = 0
 * would print:
 *      [i:0,j:1,k:2] = 00000000 00000000 00000000 00000000 (0)
 */
#define du_reflect3D(tensor, x, y, z)                                         \
    std::cout << "[" << #x << ":" << x << ", " << #y << ":" << y << ", "      \
              << #z << ":" << z << "]"                                        \
              << " = " << myutils::printing::toBinary(tensor->at(x, y, z))    \
              << " (" << tensor->at(x, y, z) << ")" << std::endl;

/**
 * @brief Macro for printing Tensor2D and reflecting their values.
 *
 * Example:
 *      int i = 0;
 *      int j = 1;
 *      reflectTensorAt(tensor, i, j); // where tensor->at(i, j) = 0
 * would print:
 *      [i:0,j:1] = 00000000 00000000 00000000 00000000 (0)
 */
#define du_reflect2D(tensor, x, y)                                            \
    std::cout << "[" << #x << ":" << x << ", " << #y << ":" << y << "]"       \
              << " = " << myutils::printing::toBinary(tensor->at(x, y))       \
              << " (" << tensor->at(x, y) << ")" << std::endl;

/**
 * @brief Macro for printing a tag. It will not print a new line.
 */
#define du_tag(tag) std::cout << "|" << tag << "| ";

/**
 * @brief Macro for printing a number of bits. It will print a new line.
 */
#define du_pbits(T)                                                           \
    std::cout << #T << ": " << sizeof(T) * CHAR_BIT << "-bits" << std::endl;

/**
 * @brief Macro for general reflection. It will not print a new line.
 */
#define du_reflect(var)                                                       \
    std::cout << #var << " = " << myutils::printing::toBinary(var) << " ("    \
              << var << ")" << std::endl;
/**
 * @brief Macro for general reflection. It will not print a new line.
 */
#define du_reflect_val(var) std::cout << #var << " = " << var << std::endl;

/**
 * @brief Macro for general reflection. Alias for du_reflect_val.
 */
#define du_rv(var) du_reflect_val(var)

/**
 * @brief Macro for general reflection. It will not print a new line.
 *
 * For every argument, it will print the argument's name and value with
 * du_reflect_val. Such that: int i = 0; int j = 1; du_reflect(i, j); would
 * print: i = 0 j = 1
 */
#define du_reflect_list(...) {0, ((void)du_reflect_val(__VA_ARGS__), 0)...};

/**
 * @brief Macro for general list reflection. Alias for du_reflect_list.
 */
#define du_rl(...) du_reflect_list(__VA_ARGS__);

        enum ALIGNMENT
        {
            LEFT,
            RIGHT,
            CENTER
        };

        /**
         * @brief General printing. It will print a new line.
         */
        namespace detail
        {
            // Number of decimal points in the output
            extern int precision;
            template <typename T> struct num_chars_impl
            {
                // For any integral type
                static size_t num_chars(uint64_t n)
                {
                    size_t num_chars = (n < 0 ? 1 : 0);
                    while (n != 0)
                    {
                        num_chars++;
                        n /= 10;
                    }
                    return (num_chars == 0 ? 1 : num_chars);
                }
                static size_t num_chars(uint32_t n)
                {
                    return num_chars((uint64_t)n);
                }
                static size_t num_chars(uint16_t n)
                {
                    return num_chars((uint64_t)n);
                }
                static size_t num_chars(uint8_t n)
                {
                    return num_chars((uint64_t)n);
                }
                static size_t num_chars(int64_t n)
                {
                    size_t num_chars = (n < 0 ? 1 : 0);
                    while (n != 0)
                    {
                        num_chars++;
                        n /= 10;
                    }
                    return (num_chars == 0 ? 1 : num_chars);
                }
                static size_t num_chars(int32_t n)
                {
                    return num_chars((int64_t)n);
                }
                static size_t num_chars(int16_t n)
                {
                    return num_chars((int64_t)n);
                }
                static size_t num_chars(int8_t n)
                {
                    return num_chars((int64_t)n);
                }
                // For any floating point type
                static size_t num_chars(float f)
                {
                    char fmt[10];
                    char str[100];
                    sprintf(fmt, "%%.%df", precision);
                    sprintf(str, fmt, f);
                    std::string s = str;
                    return s.length();
                }
                static size_t num_chars(double f)
                {
                    char fmt[10];
                    char str[100];
                    sprintf(fmt, "%%.%df", precision);
                    sprintf(str, fmt, f);
                    std::string s = str;
                    // printf("%s, len=%ld, percision=%d\n", str, s.length(),
                    //        precision);
                    return s.length();
                }
                // For any string type
                static size_t num_chars(std::string s) { return s.length(); }
                static size_t num_chars(char* s)
                {
                    return num_chars(std::string(s));
                }
                static size_t num_chars(const char* s)
                {
                    return num_chars(std::string(s));
                }
            };

            template <typename T> struct print_char_impl
            {
                // For any integer type
                static void print_char(uint64_t d) { std::cout << d; }
                static void print_char(uint32_t d) { std::cout << d; }
                static void print_char(uint16_t d) { std::cout << d; }
                static void print_char(uint8_t d) { std::cout << d; }
                static void print_char(int64_t d) { std::cout << d; }
                static void print_char(int32_t d) { std::cout << d; }
                static void print_char(int16_t d) { std::cout << d; }
                static void print_char(int8_t d) { std::cout << d; }
                // For any floating point type
                static void print_char(float f)
                {
                    char fmt[10];
                    char str[100];
                    sprintf(fmt, "%%.%df", precision);
                    sprintf(str, fmt, f);
                    std::string s = str;
                    std::cout << s;
                }
                static void print_char(double f)
                {
                    char fmt[10];
                    char str[100];
                    sprintf(fmt, "%%.%df", precision);
                    sprintf(str, fmt, f);
                    std::string s = str;
                    std::cout << s;
                }
                // For any string type
                static void print_char(std::string s) { std::cout << s; }
                static void print_char(char* s) { std::cout << s; }
                static void print_char(const char* s) { std::cout << s; }
            };

            template <typename T>
            static void printImpl(const char* color, const int max_padding,
                                  const ALIGNMENT padding_type, const T& o)
            {
                int padding = max_padding - num_chars_impl<T>::num_chars(o);
                int padding_left  = 0;
                int padding_right = 0;
                switch (padding_type)
                {
                    case ALIGNMENT::LEFT:
                        padding_right = padding;
                        break;
                    case ALIGNMENT::RIGHT:
                        padding_left = padding;
                        break;
                    case ALIGNMENT::CENTER:
                        padding_left  = padding / 2;
                        padding_right = padding - padding_left;
                        break;
                }
                for (int i = 0; i < padding_left; i++)
                {
                    std::cout << " ";
                }
                std::cout << color;
                detail::print_char_impl<T>::print_char(o);
                std::cout << RESET;
                for (int i = 0; i < padding_right; i++)
                {
                    std::cout << " ";
                }
            }

            /* template <typename T>
            static void printImpl(const char *color, const int max_padding,
                                  const ALIGNMENT padding_type, const
            Tensor3D<T> &tensor) { char sep = '['; for (const auto
            &inner_tensor : tensor) { std::cout << GRAY << sep << RESET;
                    printImpl(color, max_padding, padding_type, inner_tensor);
                    sep = ',';
                }
                std::cout << GRAY << "]" << RESET;
            } */
        } // namespace detail

        /**
         * @brief Retrive the max number of characters in a list of Ts.
         */
        template <typename T> static int getMaxPadding(T* list, int size)
        {
            int max_padding = 0;
            int padding     = 0;
            for (int i = 0; i < size; i++)
            {
                padding = detail::num_chars_impl<T>::num_chars(list[i]);
                if (padding > max_padding)
                {
                    max_padding = padding;
                }
            }
            return max_padding;
        }

        /**
         * @brief Retrive the max number of characters per column in a 2D list
         * of Ts.
         */
        template <typename T>
        static size_t* getMaxPadding(T* list, int width, int height)
        {
            size_t* max_padding = new size_t[width];
            size_t  padding     = 0;
            for (int i = 0; i < width; i++)
            {
                max_padding[i] = 0;
            }
            for (int j = 0; j < height; j++)
            {
                for (int i = 0; i < width; i++)
                {
                    padding = detail::num_chars_impl<T>::num_chars(
                        list[j * width + i]);
                    if (padding > max_padding[i])
                    {
                        max_padding[i] = padding;
                    }
                }
            }
            return max_padding;
        }

        template <typename... Args>
        static void print(const char* color, const int max_padding,
                          const ALIGNMENT padding_type, Args&&... args)
        {
// Check if fold expressions are supported
#if __cplusplus >= 201703L
            (detail::printImpl(color, max_padding, padding_type,
                               std::forward<Args>(args)),
             ...);
#else
            // Alternatively, if you don't have C++17 fold expressions:
            int dummy[] = {
                0, ((void)detail::printImpl(color, max_padding, padding_type,
                                            std::forward<Args>(args)),
                    0)...};
            (void)dummy;
#endif
        }

        template <typename... Args>
        static void printn(const char* color, const int max_padding,
                           const ALIGNMENT padding_type, Args&&... args)
        {
            myutils::printing::print(color, max_padding, padding_type,
                                     std::forward<Args>(args)...);
            std::cout << RESET << std::endl;
        }

/**
 * @brief Macro for general printing. It WILL print a new line.
 */
#define du_pprint(...)                                                        \
    myutils::printing::printn(RESET, 0, myutils::printing::ALIGNMENT::LEFT,   \
                              __VA_ARGS__);

/**
 * @brief Macro for general color printing. It WILL print a new line.
 */
#define du_pprintc(color, ...)                                                \
    myutils::printing::printn(color, 0, myutils::printing::ALIGNMENT::LEFT,   \
                              __VA_ARGS__);
/**
 * @brief Macro for general color printing. It WILL print a new line.
 */
#define du_pprintc_cond(color1, color2, cond, ...)                            \
    if (cond)                                                                 \
        myutils::printing::printn(                                            \
            color1, 0, myutils::printing::ALIGNMENT::LEFT, __VA_ARGS__);      \
    else                                                                      \
        myutils::printing::printn(                                            \
            color2, 0, myutils::printing::ALIGNMENT::LEFT, __VA_ARGS__);
/**
 * @brief Macro for general color printing. It WILL print a new line.
 */
#define du_pprintcnz(color1, color2, val)                                     \
    if (val)                                                                  \
        myutils::printing::printn(color1, 0,                                  \
                                  myutils::printing::ALIGNMENT::LEFT, val);   \
    else                                                                      \
        myutils::printing::printn(color2, 0,                                  \
                                  myutils::printing::ALIGNMENT::LEFT, val);

/**
 * @brief Macro for general printing. It will NOT print a new line.
 */
#define du_pprint_n(...)                                                      \
    myutils::printing::print(RESET, 0, myutils::printing::ALIGNMENT::LEFT,    \
                             __VA_ARGS__);

/**
 * @brief Macro for general color printing. It will NOT print a new line.
 */
#define du_pprintc_n(color, ...)                                              \
    myutils::printing::print(color, 0, myutils::printing::ALIGNMENT::LEFT,    \
                             __VA_ARGS__);

/**
 * @brief Macro for general color printing. It will NOT print a new line.
 */
#define du_pprintcp_n(color, padding, padding_type, ...)                      \
    myutils::printing::print(color, padding, padding_type, __VA_ARGS__);

/**
 * @brief Macro for general color printing. It will NOT print a new line.
 */
#define du_pprintc_cond_n(color1, color2, cond, ...)                          \
    if (cond)                                                                 \
        myutils::printing::print(                                             \
            color1, 0, myutils::printing::ALIGNMENT::LEFT, __VA_ARGS__);      \
    else                                                                      \
        myutils::printing::print(                                             \
            color2, 0, myutils::printing::ALIGNMENT::LEFT, __VA_ARGS__);

/**
 * @brief Macro for general color printing. It will NOT print a new line.
 */
#define du_pprintcnz_n(color1, color2, val)                                   \
    if (val)                                                                  \
        myutils::printing::print(color1, 0,                                   \
                                 myutils::printing::ALIGNMENT::LEFT, val);    \
    else                                                                      \
        myutils::printing::print(color2, 0,                                   \
                                 myutils::printing::ALIGNMENT::LEFT, val);

/**
 * @brief Macro for general color printing. It will NOT print a new line.
 */
#define du_pprintcnzp_n(color1, color2, padding, padding_type, val)           \
    if (val)                                                                  \
        myutils::printing::print(color1, padding, padding_type, val);         \
    else                                                                      \
        myutils::printing::print(color2, padding, padding_type, val);

    } // namespace printing

} // namespace myutils

#endif // INCLUDE_CUSTOM_UTILS_PRINT_HH_
