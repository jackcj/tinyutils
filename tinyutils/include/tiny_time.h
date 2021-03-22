#ifndef TINY_TIME_H
#define	TINY_TIME_H

#include <ctime>
#include <stdint.h>
#include <chrono>
#include <string>
#include <algorithm>
struct timeval;
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	extern void tm_gmtime(time_t* t, struct tm* bdt);
	extern void tm_localtime(time_t* t, struct tm* bdt);
	extern void tm_localtime_now(struct tm* bdt);
	extern void tm_gmtime_now(struct tm* bdt);
	extern int clock_real_time(struct timespec* tp);
	extern int clock_mono_time(struct timespec* tp);
#ifdef __cplusplus
}
#endif /* __cplusplus */


#include "tiny_thread.h"
#include <map>
namespace tiny
{
    namespace time_detail
    {
        using std::chrono::duration_cast;
        using std::chrono::seconds;
        using std::chrono::microseconds;
        using std::chrono::nanoseconds;
        // Currently we use a 64-bit count of nanoseconds.

        // We could, if we wished, use a struct holding a uint64_t count
        // of seconds and a uint32_t count of nanoseconds.

        // At least this way we can change it to something else if we
        // want.
        typedef uint64_t rep;

        // A concrete duration, unsigned. The timespan Ceph thinks in.
        typedef std::chrono::duration<rep, std::nano> timespan;


        // Like the above but signed.
        typedef int64_t signed_rep;

        typedef std::chrono::duration<signed_rep, std::nano> signedspan;

        class real_clock {
        public:
            typedef timespan duration;
            typedef duration::rep rep;
            typedef duration::period period;
            // The second template parameter defaults to the clock's duration
            // type.
            typedef std::chrono::time_point<real_clock> time_point;
            static constexpr const bool is_steady = false;

            static real_clock::time_point now() noexcept;

            static bool is_zero(const time_point& t) {
                return (t == time_point::min());
            }

            static time_point zero() {
                return time_point::min();
            }

            // Allow conversion to/from any clock with the same interface as
            // std::chrono::system_clock)
            template<typename Clock, typename Duration>
            static time_point to_system_time_point(
                const std::chrono::time_point<Clock, Duration>& t) {
                return time_point(seconds(Clock::to_time_t(t)) +
                    duration_cast<duration>(t.time_since_epoch() %
                        seconds(1)));
            }
            template<typename Clock, typename Duration>
            static std::chrono::time_point<Clock, Duration> to_system_time_point(
                const time_point& t) {
                return (Clock::from_time_t(to_time_t(t)) +
                    duration_cast<Duration>(t.time_since_epoch() % seconds(1)));
            }

            static time_t to_time_t(const time_point& t) noexcept {
                return duration_cast<seconds>(t.time_since_epoch()).count();
            }
            static time_point from_time_t(const time_t& t) noexcept {
                return time_point(seconds(t));
            }

            static void to_timespec(const time_point& t, struct timespec& ts) {
                ts.tv_sec = to_time_t(t);
                ts.tv_nsec = static_cast<long>(duration_cast<nanoseconds>(t.time_since_epoch() % seconds(1)).count());
            }
            static struct timespec to_timespec(const time_point& t) {
                struct timespec ts;
                to_timespec(t, ts);
                return ts;
            }
            static time_point from_timespec(const struct timespec& ts) {
                return time_point(seconds(ts.tv_sec) + nanoseconds(ts.tv_nsec));
            }


            static void to_timeval(const real_clock::time_point& t, struct timeval& tv);
            static real_clock::time_point from_timeval(const struct timeval& tv);

            static double to_double(const time_point& t) {
                return std::chrono::duration<double>(t.time_since_epoch()).count();
            }
            static time_point from_double(const double d) {
                return time_point(duration_cast<duration>(
                    std::chrono::duration<double>(d)));
            }
        };


        class mono_clock {
        public:
            typedef timespan duration;
            typedef duration::rep rep;
            typedef duration::period period;
            typedef std::chrono::time_point<mono_clock> time_point;
            static constexpr const bool is_steady = true;

            static mono_clock::time_point now() noexcept;

            static bool is_zero(const time_point& t) {
                return (t == time_point::min());
            }
            static time_t to_time_t(const time_point& t) noexcept {
                return duration_cast<seconds>(t.time_since_epoch()).count();
            }
            static void to_timespec(const time_point& t, struct timespec& ts) {
                ts.tv_sec = to_time_t(t);
                ts.tv_nsec = static_cast<long>(duration_cast<nanoseconds>(t.time_since_epoch() % seconds(1)).count());
            }
            static struct timespec to_timespec(const time_point& t) {
                struct timespec ts;
                to_timespec(t, ts);
                return ts;
            }
            static time_point zero() {
                return time_point::min();
            }
            static double to_double(const time_point& t) {
                return std::chrono::duration<double>(t.time_since_epoch()).count();
            }
            static time_point from_double(const double d) {
                return time_point(duration_cast<duration>(
                    std::chrono::duration<double>(d)));
            }
        };
    }

    class utime_t
    {
    public:
        utime_t();
        utime_t(time_t s, long n);
        utime_t(const struct timespec& v);
        utime_t(const struct timeval& v);
        utime_t(const struct timeval* v);
        utime_t(double secs);
        utime_t(const time_detail::real_clock::time_point& tp);
        utime_t(const time_detail::mono_clock::time_point& tp);
        void to_timespec(struct timespec* ts) const;
        void set_from_double(double d);
        time_detail::real_clock::time_point to_real_time() const;
    public:
        bool operator ! () const;
        bool operator != (const utime_t& t) const;
        bool operator == (const utime_t& t) const;
        bool operator < (const utime_t& t) const;
        bool operator > (const utime_t& t) const;
        bool operator <= (const utime_t& t) const;
        bool operator >= (const utime_t& t) const;

        utime_t& operator = (const utime_t& t);
        utime_t& operator = (double secs);

        utime_t& operator += (const utime_t& t);
        utime_t& operator -= (const utime_t& t);
        utime_t& operator += (double secs);
        utime_t& operator -= (double secs);

        utime_t operator + (const utime_t& t);
        utime_t operator - (const utime_t& t);
        utime_t operator + (double secs);
        utime_t operator - (double secs);

        // cast to double
        operator double() const {
            return (double)seconds() + ((double)nanoseconds() / 1000000000.0L);
        }
        operator struct timespec() const {
            struct timespec ts;
            ts.tv_sec = seconds();
            ts.tv_nsec = nanoseconds();
            return ts;
        }
    public:
        bool is_zero() const {
            return (tv.tv_sec == 0) && (tv.tv_nsec == 0);
        }
        void set_from_timeval(const struct timeval* v);
        void set_from_timespec(const struct timespec* v);
        void set_from_timespec(const struct timespec& v);
        int weekday();
        time_t seconds() const { return (time_t)tv.tv_sec; }
        long microseconds() const { return (tv.tv_nsec / 1000); }
        long milliseconds() const { return (tv.tv_nsec / 1000000); }
        long nanoseconds() const { return tv.tv_nsec; }
        uint64_t to_microseconds() const { return (uint64_t)tv.tv_nsec / 1000000ull + (uint64_t)tv.tv_sec * 1000000ull; }
        uint64_t to_milliseconds() const { return (uint64_t)tv.tv_nsec / 1000000ull + (uint64_t)tv.tv_sec * 1000ull; }
        uint64_t to_nanoseconds() const { return (uint64_t)tv.tv_nsec + (uint64_t)tv.tv_sec * 1000000000ull; }
        long to_days() const { return (seconds() / (24 * 3600)); }
        utime_t round_to_minute();

        utime_t round_to_hour();

        utime_t round_to_day();

        void to_tm(struct tm* t);
        std::string to_gmtime() const;
        std::string to_string(bool short_style = true) const;
        int snprintf_t(char* buf, size_t len);
        void set_date(const std::string& date);     //format : xxxx-xx-xx
        void clear();
        std::chrono::system_clock::time_point to_system_timepoint() const
        {
            return std::chrono::system_clock::time_point(std::chrono::microseconds(tv.tv_sec * 1000000000 + tv.tv_nsec));
        }
    public:
        static utime_t from_utc(const std::string& time);
        static utime_t now();
    private:
        void normalize()
        {
            if (tv.tv_nsec > 1000000000ul) {
                tv.tv_sec = cap_to_u32_max(tv.tv_sec + tv.tv_nsec / (1000000000ul));
                tv.tv_nsec %= 1000000000ul;
            }
        }
        inline uint32_t cap_to_u32_max(uint64_t t) {
            return std::min(t, (uint64_t)std::numeric_limits<uint32_t>::max());
        }
    private:
        struct {
            uint32_t tv_sec;
            uint32_t tv_nsec;
        } tv;
    };


    /*
        class Timer
    */

    class Timer
    {
    public:
        class Context
        {
        public:
            Context();
            virtual ~Context();
        public:
            void DeleteAfter();
        protected:
            virtual double OnTimeout(const utime_t& time) = 0;
        private:
            bool delete_after_;
            friend class Timer;
        };
    public:
        Timer();
        ~Timer();
    public:
        void Start();
        void Stop();
        Timer::Context* Add(double seconds, Timer::Context* callback);
        Timer::Context* Add(Timer::Context* callback, const utime_t& when);

        /* Cancel an event.
         * Call with the event_lock LOCKED
         *
         * Returns true if the callback was cancelled.
         * Returns false if you never added the callback in the first place.
         */
        bool Cancel(Timer::Context* callback = nullptr);
        void Run();
    private:
        std::thread* thread_;
        bool is_stop_;
        std::mutex mutex_;
        std::condition_variable cond_;
        std::multimap<utime_t, Timer::Context*> schedule_;
        std::map<Timer::Context*, std::multimap<utime_t, Timer::Context*>::iterator> events_;
    };
}
extern std::ostream& operator<<(std::ostream& m, const tiny::time_detail::timespan& t);
extern std::ostream& operator<<(std::ostream& m, const tiny::time_detail::mono_clock::time_point& tp);
extern std::ostream& operator<<(std::ostream& m, const tiny::time_detail::real_clock::time_point& tp);
extern std::ostream& operator<<(std::ostream& m, const tiny::utime_t& t);
#endif // !TINY_TIME_H
