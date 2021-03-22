#include "tiny_logger.h"

#include "tiny_assert.h"
#include "tiny_file.h"
#ifndef UNI_WIN
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#endif // !UNI_WIN

namespace tiny
{
	/*
	*	class SubsysMap
	*/

	class SubsysMap
	{
	public:
		class Item
		{
		public:
			Item()
				: level(5)
				, gather(5)
				, name("None")
			{

			}
			Item(int i, int g, const std::string& n)
				: level(i)
				, gather(g)
				, name(n) {}
			Item(const Item& i)
			{
				level = i.level;
				gather = i.gather;
				name = i.name;
			}
			Item(Item&& i) noexcept
			{
				std::swap(level, i.level);
				std::swap(gather, i.gather);
				std::swap(name, i.name);
			}
			Item& operator = (const Item& i)
			{
				level = i.level;
				gather = i.gather;
				name = i.name;
				return *this;
			}
			Item& operator = (Item&& i) noexcept
			{
				std::swap(level, i.level);
				std::swap(gather, i.gather);
				std::swap(name, i.name);
				return *this;
			}
			bool should_gather(int pri) const
			{
				return ((pri < level) || (pri < gather));
			}
		public:
			int level;
			int gather;
			std::string name;
		};

		using ItemMap = std::unordered_map<std::string, int>;
		using lock_guard = std::lock_guard<std::mutex>;
	public:
		const SubsysMap::Item& item(int i)
		{
			lock_guard l(mutex_);
			size_t subsys = static_cast<size_t>(i);
			if (subsys >= items_.size())
			{
				return defa_item_;
			}
			return items_[subsys];
		}
		int should_gather_log(const std::string& n, int pri)
		{
			lock_guard l(mutex_);
			ItemMap::iterator iter = subsys_.find(n);
			if (iter != subsys_.end())
			{
				if (items_[iter->second].should_gather(pri))
				{
					return iter->second;
				}
			}
			return -1;
		}

		void add(const std::string& n, int level, int gather)
		{
			if (n.empty())
			{
				return;
			}
			lock_guard l(mutex_);
			ItemMap::iterator iter = subsys_.find(n);
			if (iter == subsys_.end())
			{
				subsys_[n] = items_.size();
				items_.push_back(Item(level, gather, n));
				return;
			}
			items_[iter->second].level = level;
			items_[iter->second].gather = gather;
		}
		void set_level(int i, int level)
		{
			lock_guard l(mutex_);
			size_t subsys = static_cast<size_t>(i);
			if (subsys < items_.size())
			{
				items_[subsys].level = level;
			}
			
		}
		void set_gather(int i, int gather)
		{
			lock_guard l(mutex_);
			size_t subsys = static_cast<size_t>(i);
			if (subsys < items_.size()) 
			{
				items_[subsys].gather = gather;
			}
		}
		void set_level(const std::string& n, int level)
		{
			lock_guard l(mutex_);
			ItemMap::iterator iter = subsys_.find(n);
			if (iter != subsys_.end())
			{
				items_[iter->second].level = level;
			}
		}
		void set_gather(const std::string& n, int gather)
		{
			lock_guard l(mutex_);
			ItemMap::iterator iter = subsys_.find(n);
			if (iter != subsys_.end())
			{
				items_[iter->second].gather = gather;
			}
		}
	private:
		std::vector<Item> items_;
		ItemMap subsys_;
		Item defa_item_;
		std::mutex mutex_;
	};

	/*
	*		class InnerLogWriterImpl
	*/

	class InnerLogWriterImpl : public Logger::WriterImpl
	{
	public:
		InnerLogWriterImpl(int save_days, const std::string& path)
			: save_days_(save_days)
		{
			if (save_days_ <= 0)
			{
				save_days_ = 30;
			}
			if (path.empty())
			{
				file_.SetPath(File::Current(), "tiny_default.log");
			}
			else
			{
				file_.SetPath(path);
			}
			lastest_open_ = file_.GetCreateTime();
		}
		~InnerLogWriterImpl() override {}
	public:
		void Write(const char* data, size_t len) override
		{
			if (!reopen())
			{
				return;
			}
			size_t pos = 0;
			size_t ws = len;
			while (pos < ws)
			{
				int ret = file_.Write((data + pos), (ws - pos));
				if (ret <= 0)
				{
					file_.Close();
					if (!reopen())
					{
						return;
					}
					pos = 0;
					ws = len;
				}
				else
				{
					pos += ret;
				}

			}
			file_.Flush();
		}
		void Print(const char* data, size_t len) override 
		{
			std::cout << stringview(data, len) << std::endl;
		}
	protected:
		bool reopen()
		{
			utime_t now = utime_t::now();
			if ((now - lastest_open_).to_days() > save_days_)
			{
				file_.Close();
				lastest_open_ = now;
			}
			else
			{
				if (!file_)
				{
					if (!file_.Open("a"))
					{
						std::cout << "open log [" << file_.Path() << "] file failed" << std::endl;
						return false;
					}
				}
			}
			if (!file_)
			{
				if (!file_.Open("w"))
				{
					std::cout << "open log [" << file_.Path() << "] file failed" << std::endl;
					return false;
				}
			}
			if (!file_)
			{
				return false;
			}
			return true;
		}
	private:
		File file_;
		int save_days_;
		utime_t lastest_open_;
	};


	/*
	*	class Logger
	*/

	class OnAppExitManager
	{
	public:
		OnAppExitManager()
			: logger(nullptr)
		{
			logger = new Logger;
		}
		~OnAppExitManager()
		{
			logger->stop();
			delete logger;
		}
		Logger* get_logger() { return logger; }
	private:
		Logger* logger;
	};
#define MAX_LOG_BUF             65536
	Logger::Logger()
		: to_stop_(false)
		, started_(false)
		, m_gather_level_(-1)
		, m_log_level_(-1)
		, m_subs(new SubsysMap)
		, new_entries_max_(80)
	{

	}
	Logger::~Logger()
	{
		delete m_subs;
	}

	Logger* Logger::Instance()
	{
		static OnAppExitManager on_appexit_manager;
		return on_appexit_manager.get_logger();
	}

	int Logger::initialize(int save_days, const std::string& path, std::unique_ptr<WriterImpl> writer_impl)
	{
		if (thread_.joinable())
		{
			return -1;
		}
		if (!writer_impl)
		{
			writer_impl_.reset(new InnerLogWriterImpl(save_days, path));
		}
		else
		{
			writer_impl_ = std::move(writer_impl);
		}

#ifdef TINY_DEBUG
		set_log_subsys("tiny_sqlite3", 5, 5);
#else
		set_log_subsys("tiny_sqlite3", 1, 1);
#endif // TINY_DEBUG


		return start();
	}

	void Logger::submit_entry(Entry&& e)
	{
		unique_lock l(queue_entry_mutex_);
		if (new_entries_.size() >= new_entries_max_)
		{
			queue_entry_cond_.wait(l, [this] { return ((new_entries_.size() < new_entries_max_) || to_stop_); });
		}
		new_entries_.emplace_back(std::move(e));
		flush_entry_cond_.notify_all();
	}

	int Logger::should_gather_log(const std::string& n, int pri)
	{
		return m_subs->should_gather_log(n, pri);
	}

	void Logger::set_log_gather(const std::string& subsys, int gather)
	{
		m_subs->set_gather(subsys, gather);
	}

	void Logger::set_log_level(const std::string& subsys, int level)
	{
		m_subs->set_level(subsys, level);
	}

	void Logger::set_log_subsys(const std::string& subsys, int log_level, int gather_level)
	{
		m_subs->add(subsys, log_level, gather_level);
	}

	void Logger::set_log_level(int l)
	{
		m_log_level_ = l;
	}

	void Logger::set_gather_level(int g)
	{
		m_gather_level_ = g;
	}

	int Logger::start()
	{
		unique_lock l(queue_entry_mutex_);
		if (started_ || thread_.joinable())
		{
			return -1;
		}
		thread_ = std::thread([this] {run(); });
		queue_entry_cond_.wait(l, [this] {return started_; });
		return ((!started_) ? -1 : 0);
	}

	void Logger::stop()
	{
		{
			lock_guard l(queue_entry_mutex_);
			if (!started_)
			{
				return;
			}
			to_stop_ = true;
			flush_entry_cond_.notify_one();
			queue_entry_cond_.notify_all();
		}
		thread_.join();
		lock_guard l(queue_entry_mutex_);
		started_ = false;
		flush(new_entries_);
		if (m_log_buf.size() > 0)
		{
			if (writer_impl_.get() != nullptr)
			{
				writer_impl_->Write(m_log_buf.data(), m_log_buf.size());
			}
		}
	}
	void Logger::run()
	{
		{
			lock_guard l(queue_entry_mutex_);
			started_ = true;
			queue_entry_cond_.notify_all();
			owner_ = std::this_thread::get_id();
		}
		unique_lock l(queue_entry_mutex_);
		while (!to_stop_)
		{
			if (new_entries_.empty())
			{
				if (m_log_buf.size() > 0)
				{
					if (writer_impl_.get() != nullptr)
					{
						writer_impl_->Write(m_log_buf.data(), m_log_buf.size());
					}
					m_log_buf.resize(0);
				}
				flush_entry_cond_.wait(l);
			}
			EntryVector t;
			t.swap(new_entries_);
			queue_entry_cond_.notify_all();
			l.unlock();
			flush(t);
			l.lock();
		}
	}
	void Logger::flush(EntryVector& t)
	{
		for (ConcreteEntry& e : t)
		{

			auto prio = e.m_prio;
			auto stamp = e.m_stamp;
			auto subsys = e.m_subsys;
			auto thread = e.m_thread;
			auto str = e.strv();
			bool should_log = false;
			bool gather_log = false;
			const SubsysMap::Item& item = m_subs->item(subsys);
			int max_log_level = item.level;
			int max_gather_level = item.gather;

			if (m_log_level_ > max_log_level)
			{
				max_log_level = m_log_level_;
			}

			if (m_gather_level_ > max_gather_level)
			{
				max_gather_level = m_gather_level_;
			}

			should_log = (e.m_prio <= max_log_level);		//print
			gather_log = (e.m_prio <= max_gather_level);

			const std::size_t cur = m_log_buf.size();
			std::size_t used = 0;
			const std::size_t allocated = e.size() + 200;
			m_log_buf.resize(cur + allocated);

			char* const start = m_log_buf.data();
			char* pos = start + cur;

			used += (std::size_t)stamp.snprintf_t(pos + used, allocated - used);
			used += (std::size_t)snprintf(pos + used, allocated - used, " <%s> thread id(0x%lx), log level(%2d) ", item.name.c_str(), (unsigned long)thread, prio);
			memcpy(pos + used, str.data(), str.size());
			used += str.size();
			pos[used] = '\0';
			tiny_assert((used + 1 /* '\n' */) < allocated);

			pos[used++] = '\n';

			if (should_log)
			{
				if (writer_impl_.get() != nullptr)
				{
					writer_impl_->Print(pos, used);
				}
			}
			if (gather_log)
			{
				m_log_buf.resize(cur + used);
				if (m_log_buf.size() > MAX_LOG_BUF)
				{
					if (writer_impl_.get() != nullptr)
					{
						writer_impl_->Write(m_log_buf.data(), m_log_buf.size());
					}
					m_log_buf.resize(0);
				}
			}
			else
			{
				m_log_buf.resize(cur);
			}

		}
	}
}