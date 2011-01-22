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

#include <queue>

// from dolphin
#include "FifoQueue.h"
#include "Thread.h"

template <typename F>
THREAD_RETURN CallAndDelete(void* param)
{
	static_cast<F*>(param)->func();
	delete param;

	return 0;
}

template <typename F>
void AsyncCall(F func)
{
	struct Func
	{
		Func(F _func) : func(_func) {}
		const F func;
	};

	Common::Thread thread(&CallAndDelete<Func>, new Func(func));
}

typedef int PriorityType;

class QueueThreadJobBase
{
public:
	QueueThreadJobBase(PriorityType _priority)
		: priority(_priority) {}

	virtual void Execute() = 0;

	const PriorityType priority;
};

template <typename F>
class QueueThreadJob : public QueueThreadJobBase
{
public:
	QueueThreadJob(F func, PriorityType priority)
		: QueueThreadJobBase(priority), m_func(func) {}

	void Execute() { m_func(); };

private:
	F const m_func;
};

template <typename F, typename A>
class QueueThreadJobArg : public QueueThreadJobBase
{
public:
	QueueThreadJobArg(F func, const A arg, PriorityType priority)
		: QueueThreadJobBase(priority), m_func(func), m_arg(arg) {}

	void Execute() { m_func(m_arg); };

private:
	A m_arg;
	F const m_func;
};

template <typename Q>
THREAD_RETURN QueueThreadFunc(void* param)
{
	auto& queue_lock = *static_cast<Q*>(param);

	while (queue_lock.do_run)
	{
		queue_lock.lock.Enter();

		if (queue_lock.queue.empty())
		{
			queue_lock.lock.Leave();
			Common::SleepCurrentThread(1);
		}
		else
		{
			auto* const job = queue_lock.queue.top();
			queue_lock.queue.pop();
			queue_lock.lock.Leave();

			job->Execute();
			delete job;
		}
	}

	return 0;
}

class CompareJobPtr
{
public:	
	bool operator()(const QueueThreadJobBase* lhs, const QueueThreadJobBase* rhs)
	{
		return lhs->priority < rhs->priority;
	}
};

// better name?
class QueueThread
{
public:
	QueueThread() : queue_lock(), thread(&QueueThreadFunc<QueueLock>, &queue_lock) {}

	~QueueThread()
	{
		queue_lock.do_run = false;
		thread.WaitForDeath();
	}

	void Clear()
	{
		queue_lock.lock.Enter();
		while (!queue_lock.queue.empty())
		{
			delete queue_lock.queue.top();
			queue_lock.queue.pop();
		}
		queue_lock.lock.Leave();
	}

	template <typename F>
	void Push(PriorityType priority, F func)
	{
		queue_lock.lock.Enter();
		queue_lock.queue.push(new QueueThreadJob<F>(func, priority));
		queue_lock.lock.Leave();
	}

	template <typename F, typename A>
	void Push(PriorityType priority, F func, A arg)
	{
		queue_lock.lock.Enter();
		queue_lock.queue.push(new QueueThreadJobArg<F, A>(func, arg, priority));
		queue_lock.lock.Leave();
	}

	struct QueueLock
	{
		QueueLock() : do_run(true) {}

		std::priority_queue<QueueThreadJobBase*, std::vector<QueueThreadJobBase*>, CompareJobPtr> queue;
		Common::CriticalSection lock;
		volatile bool do_run;

	} queue_lock;

private:
	Common::Thread thread;
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
