#ifndef TINY_EVENT_CENTER_H
#define	TINY_EVENT_CENTER_H

#include "tiny_assert.h"
#include "tiny_time.h"
#include "tiny_locker.h"
#include <condition_variable>
#include <deque>
#include <thread>
#include <map>
#include <atomic>
namespace tiny
{
#define EVENT_NONE 0
#define EVENT_READABLE 1
#define EVENT_WRITABLE 2

	class EventCenter;
	/*
	*	class EventCallback
	*/
	class EventCallback {

	public:
		EventCallback(bool delete_need = false):need_delete(delete_need){}
		virtual ~EventCallback() {}       // we want a virtual destructor!!!
	public:
		virtual void do_request(uint64_t id) = 0;
		bool delete_after() const { return need_delete; }
	private:
		bool need_delete;
	};

	using EventCallbackRef = EventCallback*;

	/*
	*	class EventCallbackPtr
	*/
	class EventCallbackPtr
	{
	public:
		EventCallbackPtr() : ref_(nullptr), delete_after_(false){}
		EventCallbackPtr(EventCallbackRef ref)
			: ref_(ref) 
			, delete_after_(false)
		{
			if (ref_)
			{
				delete_after_ = ref_->delete_after();
			}
		}

		EventCallbackPtr(const EventCallbackPtr&) = delete;
		EventCallbackPtr(EventCallbackPtr&& ref) noexcept
			: ref_(nullptr)
			, delete_after_(false)
		{
			std::swap(ref_, ref.ref_);
			std::swap(delete_after_, ref.delete_after_);
		}
		~EventCallbackPtr()
		{
			reset();
		}
	public:
		EventCallbackPtr& operator = (const EventCallbackPtr&) = delete;
		EventCallbackPtr& operator = (EventCallbackPtr&& ref)  noexcept
		{
			std::swap(ref_, ref.ref_);
			std::swap(delete_after_, ref.delete_after_);
			return *this;
		}
		EventCallbackRef operator -> () { return ref_; }
		bool operator ! () const { return (!ref_); }
		operator bool() const { return (ref_ != nullptr); }
		void reset(EventCallbackRef ref = nullptr)
		{
			if (ref_)
			{
				if (delete_after_)
				{
					delete ref_;
				}
			}
			ref_ = ref;
		}
	private:
		EventCallbackRef ref_;
		bool delete_after_;
	};

	struct FiredFileEvent {
		int fd;
		int mask;
	};

	/*
	* EventDriver is a wrap of event mechanisms depends on different OS.
	* For example, Linux will use epoll(2), BSD will use kqueue(2) and select will
	* be used for worst condition.
	*/
	class EventDriver {
	public:
		virtual ~EventDriver() {}       // we want a virtual destructor!!!
		virtual int init(EventCenter *center, int nevent) = 0;
		virtual int add_event(int fd, int cur_mask, int mask) = 0;
		virtual int del_event(int fd, int cur_mask, int del_mask) = 0;
		virtual int event_wait(std::vector<FiredFileEvent> &fired_events, struct timeval *tp) = 0;
		virtual int resize_events(int newsize) = 0;
		virtual bool need_wakeup() { return true; }
	};

	/*
	*	class EventCenter
	*/

	class EventCenter
	{
	public:
		using clock_type = time_detail::mono_clock;
	private:
		struct TimeEvent {
			uint64_t id;
			EventCallbackRef time_cb;

			TimeEvent() : id(0), time_cb(NULL) {}
		};
	public:
		EventCenter();
		virtual ~EventCenter();
	public:
		int set_owner();
		void unset_owner();
		const std::thread::id& get_owner() const { return owner; }
		bool in_thread() const;
		void notify();
		uint64_t create_time_event(uint64_t microseconds, EventCallbackRef ctxt);
		void delete_time_event(uint64_t id);
		int process_events(unsigned timeout_microseconds);
		void dispatch_event_external(EventCallbackRef e);
	protected:
		virtual int initialize() { return 0; }
		virtual void uninitialize() {}
		virtual void wakeup() = 0;
		virtual int event_wait(struct timeval* tv) = 0;
	private:
		int process_time_events();
	private:
		template <typename func>
		class C_submit_event : public EventCallback {
			std::mutex lock;
			std::condition_variable cond;
			bool done = false;
			func f;
			bool nonwait;
		public:
			C_submit_event(func&& _f, bool nowait)
				: f(std::move(_f)), nonwait(nowait) {}
			void do_request(uint64_t id) override {
				f();

				lock_guard l(lock);
				cond.notify_all();
				done = true;
			}
			bool delete_after() const
			{
				return nonwait;
			}
			void wait() {
				tiny_assert(!delete_after());
				unique_lock l(lock);
				while (!done)
					cond.wait(l);
			}
		};

	public:
		template <typename func>
		void submit_to(func&& f, bool always_async = false) {
			EventCenter* c = this;
			if (always_async) {
				C_submit_event<func>* event = new C_submit_event<func>(std::move(f), true);
				c->dispatch_event_external(event);
			}
			else if (c->in_thread()) {
				f();
				return;
			}
			else {
				C_submit_event<func> event(std::move(f), false);
				c->dispatch_event_external(&event);
				event.wait();
			}
		};
	private:
		uint64_t time_event_next_id;
		std::thread::id owner;
		std::mutex external_lock;
		std::atomic_ulong external_num_events;
		std::deque<EventCallbackRef> external_events;
		std::multimap<clock_type::time_point, TimeEvent> time_events;
		std::map<uint64_t, std::multimap<clock_type::time_point, TimeEvent>::iterator> event_map;
	};
}
#endif // !TINY_EVENT_CENTER_H
