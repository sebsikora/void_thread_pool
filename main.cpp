#include "void_thread_pool.hpp"		// VoidThreadPool.

#include <functional>			// std::bind.
#include <vector>				// std::vector.
#include <iostream>				// std::cout, std::endl.
#include <sstream>				// std::stringstream.
#include <thread>				// std::thread, std::this_thread::sleep_for.
#include <chrono>				// std::chrono::seconds.
#include <cstdlib>				// rand().

class Foo {
	private:
		int m_id = 0;
	public:
		Foo(int id) { m_id = id; }
		~Foo() { }
		void Bar(void) {
			std::string message = "Foo number " + std::to_string(m_id) + " checking-in.\n";
			std::cout << message;
			int bar_delay_seconds = (rand() % 5) + 2;
			std::this_thread::sleep_for(std::chrono::seconds(bar_delay_seconds));
			message = "Foo number " + std::to_string(m_id) + " checking-out.\n";
			std::cout << message;
		}
};

int main() {
	
	VoidThreadPool thread_pool(true);
	
	std::vector<Foo*> foos = {};
	for (int i = 0; i < 4; i ++) {
		foos.push_back(new Foo(i));
	}
	
	for (int i = 0; i < 4; i ++) {
		thread_pool.AddJob(std::bind(&Foo::Bar, foos[i]));
	}
	
	thread_pool.WaitForAllJobs();
	
	thread_pool.StopPool();
	
	for (int i = 0; i < 4; i ++) {
		delete foos[i];
	}
	
	return 0;
}
