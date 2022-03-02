/******************************************************************************/
/*!
\file   ThreadPool.h
\author Jonathan Caceres / Iñigo Arenas
\par    email: jonathan.caceres@digipen.edu
\par    DigiPen login: jonathan.caceres / arenas.f@digipen.edu
\par    Course: CS180
\par    Assignment #Final
\date   12/22/2019
*/
/******************************************************************************/
#pragma once
#include<thread>
#include<vector>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<future>


/* ThreadPool class
It is a singleton. To prevent spawning
tons of threads, I made it a singleton */
class ThreadPoolClass
{
public:
	//real constructor that is used
	ThreadPoolClass()
	{
		std::size_t numThreads = std::thread::hardware_concurrency() - 1;
		mThreads.reserve(numThreads);

		for (int i = 0; i != numThreads; ++i)
		{
			mThreads.emplace_back(std::thread(&ThreadPoolClass::WorkerUpdate, this));
		}
	}

	//getInstance to allow the second constructor to be called
	static ThreadPoolClass& Instance()
	{
		static ThreadPoolClass instance;
		return instance;
	}

	//all threads have fished their job
	bool tasks_finished() { return nWaiting==mThreads.size(); }

	void ShutDown()
	{
		isStop = true;
		condition.notify_all();

		//for (std::thread& th : mThreads) { th.join(); }
		for_each(mThreads.begin(), mThreads.end(), [](std::thread& th)
			{
				th.join();
			});
		mThreads.clear();

		for (Worker* worker : mWorkers) { delete worker; }
		mWorkers.clear();
	}
	//add any arg # function to queue
	template <typename Func, typename... Args >
	void push(Func&& f, Args&&... args) 
	{
		{
			// lock jobqueue mutex, add job to the job queue 
			std::unique_lock<std::mutex> lock(queue_mutex);

			//place the job into the queue
			mWorkers.push_back(new Worker(std::move(std::bind(std::forward<Func>(f), std::forward<Args>(args)...))));
		}
		//notify a thread that there is a new job
		condition.notify_one();
	}

private:

	ThreadPoolClass(const ThreadPoolClass& other) = delete;
	ThreadPoolClass& operator=(ThreadPoolClass& other) = delete;

	//template <typename RetType>
	class Worker 
	{
	private:
		std::function<void()> func;
	public:
		Worker(std::function<void()> func) : func(std::move(func)) {}
		void execute() { func();}
	};
	// end member classes

	std::vector<std::thread>						mThreads; //the actual thread pool
	std::vector<Worker*>							mWorkers;
	std::condition_variable							condition;// used to notify threads about available jobs
	std::mutex										queue_mutex; // used to push/pop jobs to/from the queue
	std::size_t										working_tasks = 0u;
	std::atomic<bool>								isStop = false;
	std::atomic<int>								nWaiting = 0;
	//end member variables

	/* infinite loop function */
	void WorkerUpdate() 
	{
		Worker* worker;
		while (true) 
		{
			{
				std::unique_lock<std::mutex> lock(queue_mutex);

				nWaiting++;
				while (mWorkers.empty() && !isStop) { condition.wait(lock); }
				nWaiting--;

				if (isStop) return;

				worker = mWorkers.back();
				mWorkers.pop_back();
			}
			worker->execute();
			delete worker;
		}
	}
};

#define ThreadPool (ThreadPoolClass::Instance())