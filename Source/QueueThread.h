/*
Copyright (c) 2010 - Wii Banner Player Project

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef WII_BNR_QUEUE_THREAD_H_
#define WII_BNR_QUEUE_THREAD_H_

#include <map>
#include <functional>

#include <thread>
#include <mutex>
#include <condition_variable>

// from dolphin
#include "FifoQueue.h"

// better name?
class QueueThread
{
public:
	typedef int priority_type;
	typedef std::function<void()> job_type;

	QueueThread()
	{
		m_thread = std::thread(std::mem_fun(&QueueThread::ThreadFunc), this);
	}

	~QueueThread()
	{
		// push an invalid job to tell the thread to die
		PushJob(std::numeric_limits<priority_type>::max(), job_type());

		m_thread.join();
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lk(m_lock);
		m_jobs.clear();
	}

	template <typename F>
	void Push(priority_type priority, F&& func)
	{
		PushJob(priority, job_type(std::forward<F>(func)));
	}

	template <typename F, typename A>
	void Push(priority_type priority, F&& func, A&& arg)
	{
		// stupid msvc can't bind a lambda properly
		std::function<void(A&&)> job(std::forward<F>(func));

		PushJob(priority, std::bind<void>(std::move(job), std::forward<A>(arg)));
	}

	void ThreadFunc()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lk(m_lock);
			m_condition.wait(lk, [this]()->bool{ return !m_jobs.empty(); });

			auto const jobit = m_jobs.begin();
			if (jobit != m_jobs.end())
			{
				job_type func = std::move(jobit->second);
				m_jobs.erase(jobit);

				lk.unlock();

				if (!func)
					break;	// done doing jobs, end thread

				func();
			}
		}
	}

private:
	void PushJob(priority_type priority, job_type&& job)
	{
		{
		std::lock_guard<std::mutex> lk(m_lock);
		m_jobs.insert(std::make_pair(priority, std::move(job)));
		}

		m_condition.notify_one();
	}

	std::multimap<priority_type, job_type, std::greater<priority_type>> m_jobs;
	std::mutex m_lock;
	std::condition_variable m_condition;
	std::thread m_thread;
};

//THREAD_RETURN ThreadPoolThreadFunc(void* param)
//{
//	auto& func_queue = *static_cast<Common::FifoQueue<QueueThreadJobBase*>*>(param);
//
//	while (true)
//	{
//		QueueThreadJobBase* job;
//		const bool pop = func_queue.Pop(job);
//
//		if (pop)
//		{
//			if (!job)
//				break;
//
//			job->Execute();
//			delete job;
//		}
//		else
//			Common::SleepCurrentThread(1);
//	}
//
//	return 0;
//}

//class ThreadPool
//{
//	typedef int JobId;
//
//	~ThreadPool()
//	{
//		foreach (auto& job_queue, job_queues)
//		{
//			job_queue.second.lock.Enter();
//			job_queue.second.queue.push_back(nullptr);
//			job_queue.second.lock.Leave();
//		}
//
//		foreach (auto& job_queue, job_queues)
//		{
//			job_queue.second.thread->WaitForDeath();
//			delete job_queue.second.thread;
//		}
//	}
//
//	template <typename F>
//	void AddJob(JobId job_id, F func)
//	{
//		auto& job_queue = job_queues[job_id];
//
//		job_queue.lock.Enter();
//		job_queue.queue.push_back(new QueueThreadJob<F>(func));
//		job_queue.lock.Leave();
//	}
//
//	template <typename F, typename A>
//	void AddJob(JobId job_id, F func, const A& arg)
//	{
//		auto& job_queue = job_queues[job_id];
//
//		job_queue.lock.Enter();
//		job_queue.queue.push_back(new QueueThreadJobArg<F, A>(func, arg));
//		job_queue.lock.Leave();
//	}
//
//private:
//
//	struct JobQueue
//	{
//		std::deque<QueueThreadJobBase*> queue;
//		Common::CriticalSection lock;
//		Common::Thread* thread;
//	};
//
//	std::map<JobId, JobQueue> job_queues;
//};

#endif
