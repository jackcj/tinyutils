#ifndef TINY_LOGGER_H
#define	TINY_LOGGER_H

#include "tiny_thread.h"
#include "tiny_string.h"
#include "tiny_time.h"
#include "tiny_locker.h"
#include <unordered_map>
#include <atomic>
namespace tiny
{
    class Entry {
    public:

        Entry() = delete;
        Entry(short pri, int subsys) :
            m_stamp(utime_t::now()),
            m_thread(Thread::posix_this_thread_id()),
            m_prio(pri),
            m_subsys(subsys)
        {}
        Entry(const Entry&) = default;
        Entry& operator=(const Entry&) = default;
        Entry(Entry&& e) = default;
        Entry& operator=(Entry&& e) = default;
        virtual ~Entry() = default;

        virtual stringview strv() const = 0;
        virtual std::size_t size() const = 0;

        utime_t m_stamp;
        unsigned long m_thread;
        short m_prio;
        int m_subsys;
    };

    /* This should never be moved to the heap! Only allocate this on the stack. See
     * CachedStackStringStream for rationale.
     */
    class MutableEntry : public Entry {
    public:
        MutableEntry() = delete;
        MutableEntry(short pri, int subsys) : Entry(pri, subsys) {}
        MutableEntry(const MutableEntry&) = delete;
        MutableEntry& operator=(const MutableEntry&) = delete;
        MutableEntry(MutableEntry&&) = delete;
        MutableEntry& operator=(MutableEntry&&) = delete;
        ~MutableEntry() override = default;

        std::ostream& get_ostream() {
            return *cos;
        }

        stringview strv() const override {
            return cos->strv();
        }
        std::size_t size() const override {
            return cos->strv().size();
        }

    private:
        CachedStackStringStream cos;
    };

    class ConcreteEntry : public Entry {
    public:
        ConcreteEntry() = delete;
        ConcreteEntry(const Entry& e) : Entry(e) {
            auto strv = e.strv();
            str.reserve(strv.size());
            str.insert(str.end(), strv.begin(), strv.end());
        }
        ConcreteEntry& operator=(const Entry& e) {
            Entry::operator=(e);
            auto strv = e.strv();
            str.reserve(strv.size());
            str.assign(strv.begin(), strv.end());
            return *this;
        }
        ConcreteEntry(ConcreteEntry&& e) noexcept
            : Entry(e), str(std::move(e.str))
        {
        }
        ConcreteEntry& operator=(ConcreteEntry&& e) noexcept
        {
            Entry::operator=(e);
            str = std::move(e.str);
            return *this;
        }
        ~ConcreteEntry() override = default;

        stringview strv() const override {
            return stringview(str.data(), str.size());
        }
        std::size_t size() const override {
            return str.size();
        }

    private:
        std::vector<char> str;
    };


    class SubsysMap;
    class OnAppExitManager;
	class Logger
	{
        using EntryVector = std::vector<ConcreteEntry>;
        static const std::size_t DEFAULT_MAX_NEW = 100;
        static const std::size_t DEFAULT_ENTRY_BUFFER_SIZE = 4096;
    public:
        class WriterImpl
        {
        public:
            virtual ~WriterImpl() {}
        public:
            virtual void Write(const char* data, size_t len) = 0;
            virtual void Print(const char* data, size_t len) = 0;
        };
	private:
		Logger();
		~Logger();
    public:
        static Logger* Instance();
    public:
        int initialize(int save_days, const std::string& path, std::unique_ptr<WriterImpl> writer_impl = std::unique_ptr<WriterImpl>());
        void submit_entry(Entry&& e);
        int should_gather_log(const std::string& n, int pri);
        void set_log_gather(const std::string& subsys, int gather);
        void set_log_level(const std::string& subsys, int level);
        void set_log_subsys(const std::string& subsys, int log_level, int gather_level);
        void set_log_level(int l);
        void set_gather_level(int g);
        bool in_thread() const { return (std::this_thread::get_id() == owner_); }
	protected:
        int start();
        void stop();
        void run();
    private:
        void flush(EntryVector& t);
	private:
        bool to_stop_;
        bool started_;
        std::atomic_int m_gather_level_;
        std::atomic_int m_log_level_;
        SubsysMap* m_subs;
        size_t new_entries_max_;
        std::mutex queue_entry_mutex_;
        std::condition_variable queue_entry_cond_;
        std::mutex flush_entry_mutex_;
        std::condition_variable flush_entry_cond_;
        EntryVector new_entries_;
        std::vector<char> m_log_buf;
        std::unique_ptr<WriterImpl> writer_impl_;
        std::thread thread_;
        std::thread::id owner_;
        friend class OnAppExitManager;
	};
}



/// <summary>
/// 使用方法
/// 1，设置控制台打印级别（可选）tiny::Logger::Instance()->set_log_level();
/// 2， 实现Logger::WriterImpl类，保存日志及打印，调用tiny::Logger::Instance()->initialize(...)，（可选，内部有默认实现）；
/// 3，增加自己的日志模块，tiny::Logger::Instance()->set_log_subsys(const std::string& subsys, int log_levl, int gather_level); subsys为模块名，打印级别和写入级别；
/// 4，在需要日志的地方包含tiny_logger.h文件，并定义自己的日志模块：
///         a):#define ldout(v)		dout_impl(v, subsys_name)
///         b):lderr()				dout_impl(-1, subsys_name)
///         d): ...
/// 5，dout(10) << "test log level" << 10 << dendl;
///    derr(cct) << ... << dendl;
/// </summary>
/// 
/// 
#define dout_impl(pri, subsys)										                                    \
		do {                                                                                            \
			int subsys_index = tiny::Logger::Instance()->should_gather_log(#subsys, pri);               \
			if (subsys_index >= 0) {                                                                    \
				tiny::MutableEntry _dout_e(pri, subsys_index);                                          \
				std::ostream* _dout = &_dout_e.get_ostream();                                           \
				*_dout

#define dendl_impl std::flush;														\
			tiny::Logger::Instance()->submit_entry(std::move(_dout_e));				\
		}																			\
	} while (0)

#define dendl					dendl_impl


#endif // !TINY_LOGGER_H
