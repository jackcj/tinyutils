#include "tiny_thread.h"

#ifdef UNI_WIN
#include <Windows.h>
#else
#include <pthread.h>
#endif // UNI_WIN
#include "tiny_assert.h"
namespace tiny
{
	Thread::~Thread()
	{
		tiny_assert(!thread_.joinable());
	}
	bool Thread::is_running() const
	{
		return thread_.joinable();
	}
	int Thread::start()
	{
		if (thread_.joinable())
		{
			return -1;
		}
		int ret = on_start();
		if (ret != 0)
		{
			return ret;
		}
		thread_ = std::thread([this] { owner_ = std::this_thread::get_id();  run(); owner_ = std::thread::id(); });
		return on_started();
	}
	void Thread::stop()
	{
		if (!is_running())
		{
			return;
		}
		on_stop();
		thread_.join();
		on_stopped();
	}
	void Thread::join()
	{
		if (!thread_.joinable())
		{
			return;
		}
		thread_.join();
	}
	unsigned long Thread::posix_this_thread_id()
	{
#ifdef UNI_WIN
		return GetCurrentThreadId();
#else
		return pthread_self();
#endif // UNI_WIN
	}



	/*
	*	class ThreadStack
	*/
	ThreadStack::ThreadStack()
		: started(false)
	{
	}

	ThreadStack::~ThreadStack()
	{
		tiny_assert(!started);
	}
	int ThreadStack::Create()
	{
		size_t size;
		if (0 != OnCreate(size))
		{
			return -1;
		}
		if (size == 0|| size==std::string::npos)
		{
			return -1;
		}
		stack_lock.lock();
		if (started)
		{
			stack_lock.unlock();
			return -1;
		}
		threads.resize(size);
		for (size_t i = 0; i < size; ++i)
		{
			std::function<void()> thread = add_thread(i);
			threads[i] = std::thread(std::move(thread));
		}
		started = true;
		stack_lock.unlock();
		for (size_t i = 0; i < size; ++i)
		{
			wait_started(i);
		}
		OnCreated();
		return 0;
	}
	void ThreadStack::Destroy()
	{
		OnDestroy();
		{
			std::lock_guard<std::mutex> l(stack_lock);
			if (!started)
			{
				return;
			}
			for (size_t i = 0; i < threads.size(); ++i)
			{
				remove_thread(i);
			}
		}

		for (size_t i = 0; i < threads.size(); ++i)
		{
			if (threads[i].joinable())
			{
				threads[i].join();
			}
		}
		started = false;
		threads.clear();
		OnDestroyed();
	}
}