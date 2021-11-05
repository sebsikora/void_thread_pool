// Ultra-simple thread pool for running void() functions in worker threads.
// © 2021, Dr Seb Sikora.
// seb.nf.sikora@protonmail.com
//
// A synthesis of the brilliantly helpful code snippets by users 'phd-ap-ece' and 'pio' on the following stackoverflow post -
// 		https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
// Useful supplementary information found here -
// 		https://stackoverflow.com/questions/10673585/start-thread-with-member-function
// Explanation of the syntax [this]() { ... } for the predicate second argument to condition_variable.wait() found here -
// 		https://stackoverflow.com/questions/39565218/c-condition-variable-wait-for-predicate-in-my-class-stdthread-unresolved-o

#ifndef VOID_TPOOL_H
#define VOID_TPOOL_H

#include <vector>						// std::vector.
#include <iostream>						// std::cout, std::endl.
#include <sstream>						// std::stringstream.
#include <thread>						// std::thread.
#include <mutex>						// std::mutex, std::unique_lock.
#include <condition_variable>			// std::condition_variable.
#include <queue>						// std::queue.
#include <functional>					// std::function.

class VoidThreadPool {
	private:
		bool m_display_messages = false;
		std::vector<std::thread> m_threads = {};
		bool m_pool_stopped = false;
		// Synchronisation of job queue and stop flag.
		std::condition_variable m_condition;
		std::mutex m_pool_lock;
		std::queue<std::function<void()>> m_job_queue = {};
		bool m_stop_pool = false;
		// Synchronisation of pending job counter and waiting flag.
		std::condition_variable m_job_counter_condition;
		std::mutex m_job_counter_lock;
		int m_jobs_pending = 0;
		bool m_waiting_for_completion = false;
		
	public:
		VoidThreadPool(bool display_messages = false);
		~VoidThreadPool();
		void WorkerFunction(int worker_id);
		void AddJob(std::function<void()> new_job);
		void WaitForAllJobs(void);
		void StopPool(void);
};

VoidThreadPool::VoidThreadPool(bool display_messages) {
	m_display_messages = display_messages;
	int num_threads = std::thread::hardware_concurrency();
	if (m_display_messages) {
		std::cout << "Void ThreadPool starting " << num_threads << " worker threads..." << std::endl;
	}
	for (int i = 0; i < num_threads; i ++) {
		// Pass worker member function by reference to avoid the thread making it's own copy.
		m_threads.push_back(std::thread(&VoidThreadPool::WorkerFunction, this, i));
	}
}

VoidThreadPool::~VoidThreadPool() {
	if (!m_pool_stopped) {
		StopPool();
	}
}

void VoidThreadPool::WorkerFunction(int worker_id) {
	if (m_display_messages) {
		// Pre-composing our whole message stops messages from one thread being interrupted by messages from another thread.
		std::string message = "Worker function " + std::to_string(worker_id) + " starting.\n";
		std::cout << message;
	}
	
	std::function<void()> job;
	while (true) {
		{
			std::unique_lock<std::mutex> lock(m_pool_lock);
			// See below for explanation of the syntax [this]() { ... } for the predicate second argument to condition_variable.wait().
			// https://stackoverflow.com/questions/39565218/c-condition-variable-wait-for-predicate-in-my-class-stdthread-unresolved-o
			m_condition.wait(lock, [this]() { return !m_job_queue.empty() || m_stop_pool; });
			
			if (m_stop_pool && m_job_queue.empty()) {
				if (m_display_messages) {
					std::string message = "Worker function " + std::to_string(worker_id) + " stoppped.\n";
					std::cout << message;
				}
				return;
			}
			
			job = m_job_queue.front();
			m_job_queue.pop();
		}
		job();

		{
			// Once the current job has finished, decrement the pending job counter.
			// If there are no pending jobs and we are waiting, alert the waiting function via condition variable.
			std::unique_lock<std::mutex> lock(m_job_counter_lock);
			m_jobs_pending --;
			if ((m_jobs_pending == 0) && (m_waiting_for_completion)) {
				m_job_counter_condition.notify_one();
			}
		}
	}
}

void VoidThreadPool::AddJob(std::function<void()> new_job) {
	// Increment the pending job counter and push the new job onto the queue.
	{
		std::unique_lock<std::mutex> lock(m_job_counter_lock);
		m_jobs_pending ++;
	}
	{
		std::unique_lock<std::mutex> lock(m_pool_lock);
		m_job_queue.push(new_job);
	}
	// Notify one worker function that there is now a pending job.
	m_condition.notify_one();
}

void VoidThreadPool::WaitForAllJobs(void) {
	if (m_display_messages) {
		std::string message = "*** Waiting for all jobs to complete...\n";
		std::cout << message;
	}
	while (true) {		// Wrap the wait on m_job_counter_condition in a while loop in case the condition wakes up spuriously.
		{
			std::unique_lock<std::mutex> lock(m_job_counter_lock);
			m_waiting_for_completion = true;
			m_job_counter_condition.wait(lock, [this]() { return m_jobs_pending == 0; });
			if (m_jobs_pending == 0) {
				m_waiting_for_completion = false;
				if (m_display_messages) {
					std::cout << "...all jobs completed. ***" << std::endl;
				}
				break;
			}
		}
	}
}

void VoidThreadPool::StopPool(void) {
	if (m_display_messages) {
		std::string shutdown_message = "StopPool() called...\n"; 
		std::cout << shutdown_message;
	}
	// Set the stop flag and notify all processes.
	{
		std::unique_lock<std::mutex> lock(m_pool_lock);
		m_stop_pool = true;
	}
	m_condition.notify_all();
	// Wait and join all threads.
	for (std::thread& thread : m_threads) {
		thread.join();
	}
	m_threads.clear();
	m_pool_stopped = true;
	if (m_display_messages) {
		std::cout << "...all worker threads are stopped." << std::endl;
	}
}

#endif
