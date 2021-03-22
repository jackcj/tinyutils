#ifndef TINY_ASSERT_H
#define	TINY_ASSERT_H

#include <assert.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <sstream>

#ifndef __STRING
# define __STRING(x) #x
#endif

#ifdef UNI_WIN
#define	__attribute__(p)
#else
#define LAST_SYSTEM_ERROR (errno)
#endif // UNI_WIN

/* Return a given error code as a string */
std::string cpp_strerror(int err);


namespace tiny
{
	/*
	* Select a function-name variable based on compiler tests, and any compiler
	* specific overrides.
	*/
#if defined(HAVE_PRETTY_FUNC)
# define __TINY_ASSERT_FUNCTION __PRETTY_FUNCTION__
#elif defined(HAVE_FUNC)
# define __TINY_ASSERT_FUNCTION __func__
#else
# define __TINY_ASSERT_FUNCTION __FUNCTION__															 ///((__const char *) 0)
#endif

	struct assert_data {
		const char* assertion;
		const char* file;
		const int line;
		const char* function;
	};

	extern void __tiny_assert_fail(const char* assertion, const char* file, int line, const char* function)
		__attribute__((__noreturn__));
	extern void __tiny_assert_fail(const assert_data& ctx)
		__attribute__((__noreturn__));

	extern void __tiny_assertf_fail(const char* assertion, const char* file, int line, const char* function, const char* msg, ...)
		__attribute__((__noreturn__));
	extern void __tiny_assert_warn(const char* assertion, const char* file, int line, const char* function);

	[[noreturn]] void __tiny_abort(const char* file, int line, const char* func,
		const std::string& msg);

	[[noreturn]] void __tiny_abortf(const char* file, int line, const char* func,
		const char* msg, ...);

#define _TINY_ASSERT_VOID_CAST static_cast<void>

#define assert_warn(expr)							\
  ((expr)								\
   ? _TINY_ASSERT_VOID_CAST (0)					\
   : __tiny_assert_warn (__STRING(expr), __FILE__, __LINE__, __TINY_ASSERT_FUNCTION))
}

/*
 * tiny_abort aborts the program with a nice backtrace.
 *
 * Currently, it's the same as assert(0), but we may one day make assert a
 * debug-only thing, like it is in many projects.
 */
#define tiny_abort(msg, ...)                                            \
  __tiny_abort( __FILE__, __LINE__, __TINY_ASSERT_FUNCTION, "abort() called")

#define tiny_abort_msg(msg)                                             \
  __tiny_abort( __FILE__, __LINE__, __TINY_ASSERT_FUNCTION, msg) 

#define tiny_abort_msgf(...)                                             \
  __tiny_abortf( __FILE__, __LINE__, __TINY_ASSERT_FUNCTION, __VA_ARGS__)

#ifdef __SANITIZE_ADDRESS__
#define tiny_assert(expr)                           \
  do {                                              \
    ((expr))                                        \
    ? _TINY_ASSERT_VOID_CAST (0)                    \
    : __tiny_assert_fail(__STRING(expr), __FILE__, __LINE__, __TINY_ASSERT_FUNCTION); \
  } while (false)
#else
#define tiny_assert(expr)							\
  do { static const tiny::assert_data assert_data_ctx = \
   {__STRING(expr), __FILE__, __LINE__, __TINY_ASSERT_FUNCTION}; \
   ((expr) \
   ? _TINY_ASSERT_VOID_CAST (0) \
   : __tiny_assert_fail(assert_data_ctx)); } while(false)
#endif

 // this variant will *never* get compiled out to NDEBUG in the future.
 // (tiny_assert currently doesn't either, but in the future it might.)
#ifdef __SANITIZE_ADDRESS__
#define tiny_assert_always(expr)                    \
  do {                                              \
    ((expr))                                        \
    ? _TINY_ASSERT_VOID_CAST (0)                    \
    : __tiny_assert_fail(__STRING(expr), __FILE__, __LINE__, __TINY_ASSERT_FUNCTION); \
  } while(false)
#else
#define tiny_assert_always(expr)							\
  do { static const tiny::assert_data assert_data_ctx = \
   {__STRING(expr), __FILE__, __LINE__, __TINY_ASSERT_FUNCTION}; \
   ((expr) \
   ? _TINY_ASSERT_VOID_CAST (0) \
   : __tiny_assert_fail(assert_data_ctx)); } while(false)
#endif

// Named by analogy with printf.  Along with an expression, takes a format
// string and parameters which are printed if the assertion fails.
#define assertf(expr, ...)                  \
  ((expr)								\
   ? _TINY_ASSERT_VOID_CAST (0)					\
   : __tiny_assertf_fail (__STRING(expr), __FILE__, __LINE__, __TINY_ASSERT_FUNCTION, __VA_ARGS__))
#define tiny_assertf(expr, ...)                  \
  ((expr)								\
   ? _TINY_ASSERT_VOID_CAST (0)					\
   : __tiny_assertf_fail (__STRING(expr), __FILE__, __LINE__, __TINY_ASSERT_FUNCTION, __VA_ARGS__))

// this variant will *never* get compiled out to NDEBUG in the future.
// (tiny_assertf currently doesn't either, but in the future it might.)
#define tiny_assertf_always(expr, ...)                  \
  ((expr)								\
   ? _TINY_ASSERT_VOID_CAST (0)					\
   : __tiny_assertf_fail (__STRING(expr), __FILE__, __LINE__, __TINY_ASSERT_FUNCTION, __VA_ARGS__))


#define TINY_CHECK(condition)				tiny_assert(condition)

#define TINY_CHECK_EQ(a, b)	TINY_CHECK((a) == (b))
#define TINY_CHECK_NE(a, b)	TINY_CHECK((a) != (b))
#define TINY_CHECK_LE(a, b)	TINY_CHECK((a) <= (b))
#define TINY_CHECK_LT(a, b)	TINY_CHECK((a) < (b))
#define TINY_CHECK_GE(a, b)	TINY_CHECK((a) >= (b))
#define TINY_CHECK_GT(a, b)	TINY_CHECK((a) > (b))


#define TINY_DCHECK(condition)				tiny_assert(condition)

#define TINY_DCHECK_EQ(a, b)	TINY_DCHECK((a) == (b))
#define TINY_DCHECK_NE(a, b)	TINY_DCHECK((a) != (b))
#define TINY_DCHECK_LE(a, b)		TINY_DCHECK((a) <= (b))
#define TINY_DCHECK_LT(a, b)	TINY_DCHECK((a) < (b))
#define TINY_DCHECK_GE(a, b)	TINY_DCHECK((a) >= (b))
#define TINY_DCHECK_GT(a, b)	TINY_DCHECK((a) > (b))

#endif // !TINY_ASSERT_H
