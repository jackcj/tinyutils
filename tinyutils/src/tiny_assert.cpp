#include "tiny_assert.h"
#include <string.h>
#include <stdarg.h>
#ifdef UNI_WIN
#include <Windows.h>
std::string cpp_strerror(int err)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);
	std::string str((const char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return str;
}
static int pthread_self()
{
	return GetCurrentThreadId();
}
#else
#include <pthread.h>
std::string cpp_strerror(int err)
{
	char buf[128] = { 0 };
	char* errmsg = nullptr;

	if (err < 0)
		err = -err;
	std::ostringstream oss;
	// strerror_r returns char * on Linux, and does not always fill buf
#ifdef STRERROR_R_CHAR_P
	errmsg = strerror_r(err, buf, sizeof(buf));
#else
	errmsg = strerror_r(err, buf, sizeof(buf));
	errmsg = buf;
#endif

	if (!errmsg)
	{
		oss << "(" << err << ") unknown";
		return oss.str();
	}
	oss << "(" << err << ") " << errmsg;
	return oss.str();
}
#endif // UNI_WIN



#include "tiny_time.h"
#include "tiny_string.h"

namespace tiny
{
	static std::string tiny_localtime_now_string()
	{
		char date[200] = { 0 };
		struct tm bdt;
		tm_localtime_now(&bdt);
		size_t ts = ::snprintf(date, sizeof(date),
			"%04d-%02d-%02d %02d:%02d:%02d",
			bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday,
			bdt.tm_hour, bdt.tm_min, bdt.tm_sec);
		return std::string(date, ts);
	}



#define	ERROR_INFO_BUFFER_MAX_SIZE				8096
	void __tiny_assert_fail(const char* assertion, const char* file, int line, const char* function)
	{
		char buf[ERROR_INFO_BUFFER_MAX_SIZE] = { 0 };
		StringBuilder sb(buf, sizeof(buf));
		sb.AppendFormat("%s: In function '%s' thread %d time %s\n"
			"%s: %d: FAILED tiny_assert(%s)\n",
			file, function, pthread_self(), tiny_localtime_now_string().c_str(),
			file, line, assertion);

#   if defined(_MSC_VER)
		std::cout << sb.str() << std::endl;
		__debugbreak();
#   elif defined (ANDROID_NDK)
		__android_log_assert("assert", "grinliz", "%s", sb.str());
#else
		std::cout << sb.str() << std::endl;
		abort();
#endif
	}
	void __tiny_assert_fail(const assert_data& ctx)
	{
		__tiny_assert_fail(ctx.assertion, ctx.file, ctx.line, ctx.function);
	}
	void __tiny_assertf_fail(const char* assertion, const char* file, int line, const char* function, const char* msg, ...)
	{
		char buf[ERROR_INFO_BUFFER_MAX_SIZE] = { 0 };
		StringBuilder sb(buf, sizeof(buf));
		sb.AppendFormat("%s: In function '%s' thread %d time %s\n"
			"%s: %d: FAILED tiny_assert(%s)\n",
			file, function, pthread_self(), tiny_localtime_now_string().c_str(),
			file, line, assertion);
		sb.AppendFormat("Assertion details: ");
		va_list args;
		va_start(args, msg);
		sb.AppendFormat(msg, args);
		va_end(args);
#   if defined(_MSC_VER)
		std::cout << sb.str() << std::endl;
		__debugbreak();
#   elif defined (ANDROID_NDK)
		__android_log_assert("assert", "grinliz", "%s", sb.str());
#else
		std::cout << sb.str() << std::endl;
		abort();
#endif
	}
	void __tiny_assert_warn(const char* assertion, const char* file, int line, const char* function)
	{
		char buf[ERROR_INFO_BUFFER_MAX_SIZE] = { 0 };
		StringBuilder sb(buf, sizeof(buf));
		sb.AppendFormat("%s: In function '%s' thread %d time %s\n"
			"%s: %d: FAILED tiny_warn(%s)\n",
			file, function, pthread_self(), tiny_localtime_now_string().c_str(),
			file, line, assertion);
		std::cout << sb.str() << std::endl;
	}
	void __tiny_abort(const char* file, int line, const char* func, const std::string& msg)
	{
		char buf[ERROR_INFO_BUFFER_MAX_SIZE] = { 0 };
		StringBuilder sb(buf, sizeof(buf));
		sb.AppendFormat("%s: In function '%s' thread %d time %s\n"
			"%s: %d: FAILED tiny_abort(%s)\n",
			file, func, pthread_self(), tiny_localtime_now_string().c_str(),
			file, line, msg.c_str());
#   if defined(_MSC_VER)
		std::cout << sb.str() << std::endl;
		__debugbreak();
#   elif defined (ANDROID_NDK)
		__android_log_assert("assert", "grinliz", "%s", sb.str());
#else
		std::cout << sb.str() << std::endl;
		abort();
#endif
	}
	void __tiny_abortf(const char* file, int line, const char* func, const char* msg, ...)
	{
		//std::stringstream tss;
		//tss << std::this_thread::get_id();
		char buf[ERROR_INFO_BUFFER_MAX_SIZE] = { 0 };
		StringBuilder sb(buf, sizeof(buf));
		sb.AppendFormat("%s: In function '%s' thread %d time %s\n"
			"%s: %d: \n",
			file, func, pthread_self(), tiny_localtime_now_string().c_str(),
			file, line);
		va_list args;
		va_start(args, msg);
		sb.AppendFormat(msg, args);
		va_end(args);
#   if defined(_MSC_VER)
		std::cout << sb.str() << std::endl;
		__debugbreak();
#   elif defined (ANDROID_NDK)
		__android_log_assert("assert", "grinliz", "%s", sb.str());
#else
		std::cout << sb.str() << std::endl;
		abort();
#endif
	}
}