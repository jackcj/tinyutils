#ifndef TINY_THREAD_H
#define	TINY_THREAD_H


#include "tiny_locker.h"
#include <thread>
#include <vector>
#include <functional>
namespace tiny
{
	class Thread
	{
	public:
		Thread() = default;
		Thread(const Thread&) = delete;
		Thread(Thread&&) = delete;
		virtual ~Thread();
	public:
		Thread& operator = (const Thread&) = delete;
		Thread& operator = (Thread&&) = delete;
	public:
		bool in_thread() const { return (std::this_thread::get_id() == owner_); }
		bool is_running() const;
		int start();
		void stop();
		void join();
		std::thread::id thread_id() const { return owner_; }
	public:
		static unsigned long posix_this_thread_id();
	protected:
		virtual int on_start() { return 0; }
		virtual void on_stop() {}
		virtual int on_started() { return 0; }
		virtual void on_stopped() {}
		virtual void run() = 0;
	private:
		std::thread thread_;
		std::thread::id owner_;
	};

	class ThreadStack
	{
	public:
		class Worker
		{
		public:
			Worker() {}
			virtual ~Worker() {}
			void init_done() {
				init_lock.lock();
				init = true;
				init_cond.notify_all();
				init_lock.unlock();
			}
			bool is_init() {
				std::lock_guard<std::mutex> l(init_lock);
				return init;
			}
			void wait_for_init() {
				unique_lock l(init_lock);
				while (!init)
					init_cond.wait(l);
			}
			void reset() {
				init_lock.lock();
				init = false;
				init_cond.notify_all();
				init_lock.unlock();
				done = false;
			}

		public:
			bool is_done() const { return done; }
			void set_done() { done = true; }
		private:
			bool done = false;
			bool init = false;
			std::mutex init_lock;
			std::condition_variable init_cond;
		};
	public:
		ThreadStack();
		virtual ~ThreadStack();
	public:
		int Create();
		void Destroy();
	protected:
		virtual int OnCreate(size_t& thread_nums) { thread_nums = 1; return 0; }
		virtual void OnCreated() {}
		virtual void OnDestroy() {}
		virtual void OnDestroyed() {}
	public:
		size_t get_thread_numbers() const { return threads.size(); }
	public:
		virtual std::function<void()> add_thread(unsigned int thread_index) = 0;
		virtual void remove_thread(unsigned int index) = 0;
		virtual void wait_started(unsigned int index) = 0;
	private:
		bool started;
		std::mutex stack_lock;
		std::vector<std::thread> threads;
	};
}
#endif // !TINY_THREAD_H

