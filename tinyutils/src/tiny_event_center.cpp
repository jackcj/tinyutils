#include "tiny_event_center.h"

#ifdef UNI_WIN
#include <WinSock2.h>
#endif // UNI_WIN

namespace tiny
{
	EventCenter::EventCenter()
		: time_event_next_id(1)
	{
	}
	EventCenter::~EventCenter()
	{

	}
	int EventCenter::set_owner()
	{
		tiny_assert((owner == std::thread::id()));
		owner = std::this_thread::get_id();
		return initialize();
	}
	void EventCenter::unset_owner()
	{
		uninitialize();
		{
			std::lock_guard<std::mutex> l(external_lock);
			while (!external_events.empty())
			{
				EventCallbackPtr e(external_events.front());
				external_events.pop_front();
				if (e)
				{
					e->do_request(0);
				}
			}
			owner = std::thread::id();
		}
		time_events.clear();
	}
	void EventCenter::notify()
	{
		wakeup();
	}
	bool EventCenter::in_thread() const
	{
		return (owner == std::this_thread::get_id());
	}
	uint64_t EventCenter::create_time_event(uint64_t microseconds, EventCallbackRef ctxt)
	{
		tiny_assert(in_thread());
		uint64_t id = time_event_next_id++;
		EventCenter::TimeEvent event;
		clock_type::time_point expire = clock_type::now() + std::chrono::microseconds(microseconds);
		event.id = id;
		event.time_cb = ctxt;
		std::multimap<clock_type::time_point, TimeEvent>::value_type s_val(expire, event);
		auto it = time_events.insert(std::move(s_val));
		event_map[id] = it;

		return id;
	}
	void EventCenter::delete_time_event(uint64_t id)
	{
		tiny_assert(in_thread());
		if (id >= time_event_next_id || id == 0)
			return;

		auto it = event_map.find(id);
		if (it == event_map.end()) 
		{
			return;
		}

		time_events.erase(it->second);
		event_map.erase(it);
	}
	int EventCenter::process_events(unsigned timeout_microseconds)
	{
		tiny_assert(in_thread());
		struct timeval tv;
		int numevents = 0;
		bool trigger_time = false;
		auto now = clock_type::now();
		clock_type::time_point shortest;
		shortest = now + std::chrono::microseconds(timeout_microseconds);
		auto it = time_events.begin();
		if (it != time_events.end() && shortest >= it->first) 
		{
			trigger_time = true;
			shortest = it->first;
			if (shortest > now) {
				timeout_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
					shortest - now).count();
			}
			else {
				shortest = now;
				timeout_microseconds = 0;
			}
		}
		tv.tv_sec = timeout_microseconds / 1000000;
		tv.tv_usec = timeout_microseconds % 1000000;


		numevents = event_wait(&tv);

		if (trigger_time)
		{
			numevents += process_time_events();
		}
			
		if (external_num_events.load()) {
			external_lock.lock();
			std::deque<EventCallbackRef> cur_process;
			cur_process.swap(external_events);
			external_num_events.store(0);
			external_lock.unlock();
			numevents += cur_process.size();
			while (!cur_process.empty()) 
			{
				EventCallbackPtr e(cur_process.front());
				cur_process.pop_front();
				e->do_request(0);
			}
		}
		return numevents;
	}
	void EventCenter::dispatch_event_external(EventCallbackRef e)
	{
		uint64_t num = 0;
		{
			external_lock.lock();
			if (external_num_events > 0 && *external_events.rbegin() == e) {
				external_lock.unlock();
				return;
			}
			external_events.push_back(e);
			num = ++external_num_events;
			external_lock.unlock();
		}
		if (num == 1 && !in_thread())
			notify();
	}
	int EventCenter::process_time_events()
	{
		tiny_assert(in_thread());
		int processed = 0;
		clock_type::time_point now = clock_type::now();

		while (!time_events.empty()) {
			auto it = time_events.begin();
			if (now >= it->first) {
				TimeEvent& e = it->second;
				EventCallbackPtr cb(e.time_cb);
				uint64_t id = e.id;
				time_events.erase(it);
				event_map.erase(id);
				processed++;
				cb->do_request(id);
			}
			else {
				break;
			}
		}
		return processed;
	}
}
