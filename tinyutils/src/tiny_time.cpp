#include "tiny_time.h"
#include <string.h>
#ifdef UNI_WIN
#include <WinSock2.h>
#include <Windows.h>
#else
#include <sys/time.h>
#endif // UNI_WIN

#include <chrono>
#include <algorithm>
#include <cmath>
#include <time.h>
#include <iostream>
#include <iomanip>

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC (1)
#endif // !CLOCK_MONOTONIC

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME (2)
#endif // !CLOCK_REALTIME


#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>

#include <ostringstream>

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000ULL
#endif

int clock_gettime(int clk_id, struct timespec* tp)
{
	if (clk_id == CLOCK_REALTIME) {
		// gettimeofday is much faster than clock_get_time
		struct timeval now;
		int ret = gettimeofday(&now, NULL);
		if (ret)
			return ret;
		tp->tv_sec = now.tv_sec;
		tp->tv_nsec = now.tv_usec * 1000L;
	}
	else {
		uint64_t t = mach_absolute_time();
		static mach_timebase_info_data_t timebase_info;
		if (timebase_info.denom == 0) {
			(void)mach_timebase_info(&timebase_info);
		}
		auto nanos = t * timebase_info.numer / timebase_info.denom;
		tp->tv_sec = nanos / NSEC_PER_SEC;
		tp->tv_nsec = nanos - (tp->tv_sec * NSEC_PER_SEC);
	}
	return 0;
}
#endif

#ifdef UNI_WIN
int clock_gettime(int clk_id, struct timespec* tp)
{
	if (clk_id == CLOCK_MONOTONIC)
	{
		ULONGLONG ms = GetTickCount64();
		tp->tv_sec = ms / 1000;
		tp->tv_nsec = (ms % 1000) * 1000000l;
	}
	if (clk_id == CLOCK_REALTIME)
	{
		SYSTEMTIME sysnow;
		FILETIME ftime;
		GetSystemTime(&sysnow);
		SystemTimeToFileTime(&sysnow, &ftime);
		ULARGE_INTEGER ui;
		ui.LowPart = ftime.dwLowDateTime;
		ui.HighPart = ftime.dwHighDateTime;

		tp->tv_sec = ((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
		tp->tv_nsec = ((LONGLONG)(ui.QuadPart - 116444736000000000) * 100);
	}

	return 0;
}
#endif // UNI_WIN

void tm_gmtime(time_t* t, tm* bdt)
{
#ifdef UNI_WIN
	gmtime_s(bdt, t);
#else
	gmtime_r(t, bdt);
#endif // UNI_WIN
}

void tm_localtime(time_t* t, tm* bdt)
{
#ifdef UNI_WIN
	localtime_s(bdt, t);
#else
	localtime_r(t, bdt);
#endif // UNI_WIN
}
void tm_localtime_now(struct tm* bdt)
{
	time_t now = time(0);
	tm_localtime(&now, bdt);
}
void tm_gmtime_now(struct tm* bdt)
{
	time_t now = time(0);
	tm_gmtime(&now, bdt);
}
int clock_real_time(struct timespec* tp)
{
	return clock_gettime(CLOCK_REALTIME, tp);
}
int clock_mono_time(struct timespec* tp)
{
	return clock_gettime(CLOCK_MONOTONIC, tp);
}


#include "tiny_assert.h"
namespace tiny
{
	namespace time_detail
	{
		/*
			class real_clock
		*/
		real_clock::time_point real_clock::now() noexcept
		{
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			return from_timespec(ts);
		}
		void real_clock::to_timeval(const real_clock::time_point& t, timeval& tv)
		{
			tv.tv_sec = to_time_t(t);
			tv.tv_usec = static_cast<long>(duration_cast<microseconds>(t.time_since_epoch() %
				seconds(1)).count());
		}
		real_clock::time_point real_clock::from_timeval(const timeval& tv)
		{
			return time_point(seconds(tv.tv_sec) + microseconds(tv.tv_usec));
		}

		/*
			class mono_clock
		*/
		mono_clock::time_point mono_clock::now() noexcept
		{
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			return time_point(seconds(ts.tv_sec) + nanoseconds(ts.tv_nsec));
		}
	}


	utime_t::utime_t()
	{
		memset(&tv, 0, sizeof(tv));
	}
	utime_t::utime_t(time_t s, long n)
	{
		tv.tv_sec = static_cast<uint32_t>(s);
		tv.tv_nsec = static_cast<uint32_t>(n);
		normalize();
	}
	utime_t::utime_t(const timespec& v)
	{
		tv.tv_sec = static_cast<uint32_t>(v.tv_sec);
		tv.tv_nsec = static_cast<uint32_t>(v.tv_nsec);
	}
	utime_t::utime_t(const timeval& v)
	{
		memset(&tv, 0, sizeof(tv));
		set_from_timeval(&v);
	}
	utime_t::utime_t(const timeval* v)
	{
		memset(&tv, 0, sizeof(tv));
		set_from_timeval(v);
	}

	utime_t::utime_t(double secs)
	{
		memset(&tv, 0, sizeof(tv));
		set_from_double(secs);
	}

	utime_t::utime_t(const tiny::time_detail::mono_clock::time_point& tp)
	{
		memset(&tv, 0, sizeof(tv));
		set_from_timespec(tiny::time_detail::mono_clock::to_timespec(tp));
	}
	utime_t::utime_t(const tiny::time_detail::real_clock::time_point& tp)
	{
		memset(&tv, 0, sizeof(tv));
		set_from_timespec(tiny::time_detail::real_clock::to_timespec(tp));
	}
	void utime_t::to_timespec(timespec* ts) const
	{
		if (!ts)
		{
			return;
		}
		ts->tv_sec = tv.tv_sec;
		ts->tv_nsec = tv.tv_nsec;
	}
	void utime_t::set_from_double(double d)
	{
		tv.tv_sec = static_cast<uint32_t>(std::trunc(d));
		tv.tv_nsec = static_cast<uint32_t>((d - (double)tv.tv_sec) * 1000000000.0);
	}
	tiny::time_detail::real_clock::time_point utime_t::to_real_time() const
	{
		struct timespec ts;
		to_timespec(&ts);
		return tiny::time_detail::real_clock::from_timespec(ts);
	}
	bool utime_t::operator!() const
	{
		return is_zero();
	}
	bool utime_t::operator!=(const utime_t& t) const
	{
		return !(operator == (t));
	}
	bool utime_t::operator==(const utime_t& t) const
	{
		if (tv.tv_sec != t.tv.tv_sec)
		{
			return false;
		}
		return (tv.tv_nsec == t.tv.tv_nsec);
	}
	bool utime_t::operator<(const utime_t& t) const
	{
		if (tv.tv_sec < t.tv.tv_sec)
		{
			return true;
		}
		if (tv.tv_sec == t.tv.tv_sec)
		{
			return (tv.tv_nsec < t.tv.tv_nsec);
		}
		return false;
	}
	bool utime_t::operator>(const utime_t& t) const
	{
		if (tv.tv_sec > t.tv.tv_sec)
		{
			return true;
		}
		if (tv.tv_sec == t.tv.tv_sec)
		{
			return (tv.tv_nsec > t.tv.tv_nsec);
		}
		return false;
	}
	bool utime_t::operator<=(const utime_t& t) const
	{
		return !(operator > (t));
	}
	bool utime_t::operator>=(const utime_t& t) const
	{
		return !(operator < (t));
	}
	utime_t& utime_t::operator=(const utime_t& t)
	{
		tv.tv_nsec = t.tv.tv_nsec;
		tv.tv_sec = t.tv.tv_sec;
		return *this;
	}
	utime_t& utime_t::operator=(double secs)
	{
		set_from_double(secs);
		return *this;
	}
	utime_t& utime_t::operator+=(const utime_t& t)
	{
		tv.tv_sec += t.tv.tv_sec;
		tv.tv_nsec += t.tv.tv_nsec;
		normalize();
		return *this;
	}
	utime_t& utime_t::operator-=(const utime_t& t)
	{
		if (tv.tv_nsec < t.tv.tv_nsec)
		{
			--tv.tv_sec;
			tv.tv_nsec += 1000000000ull;
		}
		tv.tv_nsec -= t.tv.tv_nsec;
		tv.tv_sec -= t.tv.tv_sec;
		normalize();
		return *this;
	}
	utime_t& utime_t::operator+=(double secs)
	{
		return operator += (utime_t(secs));
	}
	utime_t& utime_t::operator-=(double secs)
	{
		return operator -= (utime_t(secs));
	}
	utime_t utime_t::operator+(const utime_t& t)
	{
		return utime_t(tv.tv_sec + t.tv.tv_sec, tv.tv_nsec + t.tv.tv_nsec);
	}
	utime_t utime_t::operator-(const utime_t& t)
	{
		uint32_t ns = tv.tv_nsec;
		uint32_t s = tv.tv_sec - t.tv.tv_sec;
		if (ns < t.tv.tv_nsec)
		{
			--s;
			ns += 1000000000ull;
		}
		ns -= t.tv.tv_nsec;
		return utime_t(s, ns);
	}
	utime_t utime_t::operator+(double secs)
	{
		return operator+(utime_t(secs));
	}
	utime_t utime_t::operator-(double secs)
	{
		return operator - (utime_t(secs));
	}
	void utime_t::set_from_timeval(const timeval* v)
	{
		if (!v)
		{
			memset(&tv, 0, sizeof(tv));
			return;
		}
		tv.tv_sec = static_cast<uint32_t>(v->tv_sec);
		tv.tv_nsec = static_cast<uint32_t>(v->tv_usec * 1000);
	}
	void utime_t::set_from_timespec(const timespec* v)
	{
		if (!v)
		{
			memset(&tv, 0, sizeof(tv));
			return;
		}
		tv.tv_sec = static_cast<uint32_t>(v->tv_sec);
		tv.tv_nsec = static_cast<uint32_t>(v->tv_nsec);
	}
	void utime_t::set_from_timespec(const timespec& v)
	{
		set_from_timespec(&v);
	}
	int utime_t::weekday()
	{
		struct tm bdt;
		to_tm(&bdt);
		return bdt.tm_wday;
	}
	utime_t utime_t::round_to_minute()
	{
		struct tm bdt;
		to_tm(&bdt);
		bdt.tm_sec = 0;
		return utime_t(mktime(&bdt), 0);
	}
	utime_t utime_t::round_to_hour()
	{
		struct tm bdt;
		to_tm(&bdt);
		bdt.tm_sec = 0;
		bdt.tm_min = 0;
		return utime_t(mktime(&bdt), 0);
	}
	utime_t utime_t::round_to_day()
	{
		struct tm bdt;
		to_tm(&bdt);
		bdt.tm_sec = 0;
		bdt.tm_min = 0;
		bdt.tm_hour = 0;
		return utime_t(mktime(&bdt), 0);
	}
	void utime_t::to_tm(tm* t)
	{
		if (!t)
		{
			return;
		}
		time_t tt = seconds();
		tm_localtime(&tt, t);
	}
	std::string utime_t::to_gmtime() const
	{
		struct tm bdt;
		time_t tt = seconds();
		tm_gmtime(&tt, &bdt);
		char date[200] = { 0 };
		size_t ts = 0;
		ts = strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &bdt);
		return std::string(date, ts);
	}
	std::string utime_t::to_string(bool short_style) const
	{
		char buf[100] = { 0 };
		size_t bs = 0;
		struct tm bdt;
		time_t tt = seconds();

		if (tt <= 10.0 * 365 * 24 * 3600)
		{
#ifdef UNI_WIN
			bs = sprintf(buf, "%lld.%ld seconds", seconds(), microseconds());
#else
			bs = sprintf(buf, "%ld.%ld seconds", seconds(), microseconds());
#endif // UNI_WIN


			return std::string(buf, bs);
		}
		tm_localtime(&tt, &bdt);

		if (short_style)
		{
			bs = snprintf(buf, sizeof(buf),
				"%04d-%02d-%02d %02d:%02d:%02d",
				bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday,
				bdt.tm_hour, bdt.tm_min, bdt.tm_sec);
		}
		else
		{
			bs = snprintf(buf, sizeof(buf),
				"%04d-%02d-%02d %02d:%02d:%02d.%06ld",
				bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday,
				bdt.tm_hour, bdt.tm_min, bdt.tm_sec, microseconds());
		}
		return std::string(buf, bs);
	}
	int utime_t::snprintf_t(char* buf, size_t len)
	{
		struct tm bdt;
		to_tm(&bdt);
		return ::snprintf(buf, len,
			"%04d-%02d-%02d %02d:%02d:%02d.%06ld",
			bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday,
			bdt.tm_hour, bdt.tm_min, bdt.tm_sec, microseconds());
	}
	void utime_t::set_date(const std::string& date)
	{
		clear();
		struct std::tm time_struct;
		memset(&time_struct, 0, sizeof(time_struct));
		int cnt = sscanf(date.c_str(), "%d-%02d-%02d", &time_struct.tm_year
			, &time_struct.tm_mon
			, &time_struct.tm_mday);
		if (cnt != 3)
		{
			return;
		}
		time_struct.tm_year -= 1900;
		time_struct.tm_mon -= 1;
		std::time_t time_unix = std::mktime(&time_struct);
		if (time_unix == -1)
		{
			return;
		}
		tv.tv_sec = time_unix;
	}
	void utime_t::clear()
	{
		memset(&tv, 0, sizeof(tv));
	}
	utime_t utime_t::from_utc(const std::string& time)
	{
		struct std::tm time_struct;
		int cnt = sscanf(time.c_str(), "%d-%02d-%02dT%02d:%02d:%02dZ", &time_struct.tm_year
			, &time_struct.tm_mon
			, &time_struct.tm_mday
			, &time_struct.tm_hour
			, &time_struct.tm_min, &time_struct.tm_sec);
		if (cnt != 6)
		{
			return utime_t();
		}
		time_struct.tm_year -= 1900;
		time_struct.tm_mon -= 1;
		std::time_t time_unix = std::mktime(&time_struct);
		if (time_unix == -1)
		{
			return utime_t();
		}
		return utime_t(time_unix, 0);
	}
	utime_t utime_t::now()
	{
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		return utime_t(ts.tv_sec, ts.tv_nsec);
	}


	/*
	*		class Timer
	*/

	Timer::Context::Context()
		: delete_after_(false)
	{
	}

	Timer::Context::~Context()
	{
	}

	void Timer::Context::DeleteAfter()
	{
		delete_after_ = true;
	}

	Timer::Timer()
		: thread_(nullptr)
		, is_stop_(false)
	{
	}

	Timer::~Timer()
	{
		Stop();
	}

	void Timer::Start()
	{
		if (thread_)
		{
			return;
		}
		is_stop_ = false;
		thread_ = new std::thread([this]() {
			this->Run();
			});
	}

	void Timer::Stop()
	{
		if (!thread_)
		{
			return;
		}
		is_stop_ = true;
		cond_.notify_all();
		thread_->join();
		delete thread_;
		thread_ = nullptr;
		Cancel();
	}

	Timer::Context* Timer::Add(double seconds, Timer::Context* callback)
	{
		utime_t now = utime_t::now();
		now += seconds;
		return Add(callback, now);
	}

	Timer::Context* Timer::Add(Timer::Context* callback, const utime_t& when)
	{
		if (is_stop_)
		{
			if (callback->delete_after_)
			{
				delete callback;
			}
			return nullptr;
		}
		std::unique_lock<std::mutex> lc(mutex_);
		std::multimap <utime_t, Timer::Context*>::value_type s_val(when, callback);
		std::multimap <utime_t, Timer::Context*>::iterator i = schedule_.insert(s_val);

		std::map < Timer::Context*, std::multimap <utime_t, Timer::Context*>::iterator >::value_type e_val(callback, i);
		std::pair < std::map < Timer::Context*, std::multimap <utime_t, Timer::Context*>::iterator >::iterator, bool > rval(events_.insert(e_val));
		/* If you hit this, you tried to insert the same Context* twice. */
		tiny_assert(rval.second);
		if (i == schedule_.begin())
			cond_.notify_all();
		return callback;
	}

	bool Timer::Cancel(Timer::Context* callback)
	{
		std::unique_lock<std::mutex> lc(mutex_);
		if (!callback)
		{
			while (!events_.empty()) {
				auto p = events_.begin();
				if (p->first->delete_after_)
				{
					delete p->first;
				}
				schedule_.erase(p->second);
				events_.erase(p);
			}
			return true;
		}

		auto p = events_.find(callback);
		if (p == events_.end()) {
			return false;
		}
		if (p->first->delete_after_)
		{
			delete p->first;
		}
		schedule_.erase(p->second);
		events_.erase(p);
		return true;
	}

	void Timer::Run()
	{

		while (!is_stop_)
		{
			utime_t now = utime_t::now();
			while (!schedule_.empty())
			{
				mutex_.lock();
				std::multimap <utime_t, Timer::Context*>::iterator p = schedule_.begin();
				if (p->first > now)
				{
					mutex_.unlock();
					break;
				}
				Timer::Context* callback = p->second;
				events_.erase(callback);
				schedule_.erase(p);
				mutex_.unlock();
				double ok = callback->OnTimeout(now);
				if (ok > 0.000001)
				{
					Add(ok, callback);
				}
				else
				{
					if (callback->delete_after_)
					{
						delete callback;
					}
				}

			}
			if (is_stop_)
			{
				break;
			}
			std::unique_lock<std::mutex> lc(mutex_);
			if (schedule_.empty())
			{
				cond_.wait(lc);
			}
			else
			{
				cond_.wait_until(lc, schedule_.begin()->first.to_system_timepoint());
			}
		}


	}
}


std::ostream& operator<<(std::ostream& m, const tiny::time_detail::timespan& t) {
	return m << std::chrono::duration<double>(t).count() << "s";
}
std::ostream& operator<<(std::ostream& m, const tiny::time_detail::mono_clock::time_point& tp) {
	return m << std::chrono::duration<double>(tp.time_since_epoch()).count() << "s";
}

std::ostream& operator<<(std::ostream& m, const tiny::time_detail::real_clock::time_point& tp)
{
	m.setf(std::ios::right);
	char oldfill = m.fill();
	m.fill('0');
	// localtime.  this looks like an absolute time.
	//  aim for http://en.wikipedia.org/wiki/ISO_8601
	struct tm bdt;
	time_t tt = tiny::time_detail::real_clock::to_time_t(tp);
	tm_localtime(&tt, &bdt);
	m << std::setw(4) << (bdt.tm_year + 1900)  // 2007 -> '07'
		<< '-' << std::setw(2) << (bdt.tm_mon + 1)
		<< '-' << std::setw(2) << bdt.tm_mday
		<< ' '
		<< std::setw(2) << bdt.tm_hour
		<< ':' << std::setw(2) << bdt.tm_min
		<< ':' << std::setw(2) << bdt.tm_sec
		<< "." << std::setw(6) << std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch() % std::chrono::seconds(1));
	m.fill(oldfill);
	m.unsetf(std::ios::right);
	return m;
}

std::ostream& operator<<(std::ostream& m, const tiny::utime_t& t)
{
	m << t.to_string(false);
	return m;
}