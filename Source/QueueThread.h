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

#include "FifoQueue.h"
#include "Thread.h"

class AsyncBase
{
public:
	virtual void Execute() = 0;
};

template <typename F>
class Async : public AsyncBase
{
public:
	Async(F func) : m_func(func) {}

	void Execute() { m_func(); };

private:
	F const m_func;
};

template <typename F, typename A>
class AsyncArg : public AsyncBase
{
public:
	AsyncArg(F func, const A& arg) : m_func(func), m_arg(arg) {}

	void Execute() { m_func(m_arg); };

private:
	A m_arg;
	F const m_func;
};

THREAD_RETURN ExecuteAsync(void* param)
{
	static_cast<AsyncBase*>(param)->Execute();
	delete param;

	return 0;
}

template <typename F>
void AsyncCall(F func)
{
	AsyncBase* const async = new Async<F>(func);
	Common::Thread thread(&ExecuteAsync, async);
	//HANDLE thread_handle = CreateThread(nullptr, 0, &ExecuteAsync, async, 0, nullptr);
	//CloseHandle(thread_handle);	// is this ok?
}

THREAD_RETURN QueueThreadFunc(void* param)
{
	auto& func_queue = *static_cast<Common::FifoQueue<AsyncBase*>*>(param);

	while (true)
	{
		AsyncBase* async;
		const bool pop = func_queue.Pop(async);

		if (pop)
		{
			if (nullptr == async)
				break;

			async->Execute();
			delete async;
		}
		else
			Common::SleepCurrentThread(1);
	}

	return 0;
}

// better name?
class QueueThread
{
public:
	QueueThread() : thread(&QueueThreadFunc, &func_queue) {}

	~QueueThread()
	{
		func_queue.Push(nullptr);
		thread.WaitForDeath();
	}

	template <typename F>
	void Push(F func)
	{
		func_queue.Push(new Async<F>(func));
	}

	template <typename F, typename A>
	void Push(F func, const A& arg)
	{
		func_queue.Push(new AsyncArg<F, A>(func, arg));
	}

private:
	Common::FifoQueue<AsyncBase*> func_queue;
	Common::Thread thread;
};

#endif
