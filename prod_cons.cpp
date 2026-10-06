#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <unistd.h>

#include "myrand.h"

namespace {

class Object {
public:
	explicit Object(int v = 0) : v_(v) {}
	~Object() = default;

	int v() const { return v_; }

private:
	int v_;
};

using MyList_t = std::queue<Object>;

MyList_t queue_;
std::mutex queue_mutex;
std::condition_variable queue_cond;
std::atomic<bool> quit(false);
constexpr size_t MaxSize = 20;

class Producer {
public:
	explicit Producer(const std::string& label) : label_(label) {}
	~Producer() = default;

	void operator()() const;

private:
	std::string label_;
};

class Consumer {
public:
	explicit Consumer(const std::string& label) : label_(label) {}
	~Consumer() = default;

	void operator()() const;

private:
	std::string label_;
};

void Producer::operator()() const {
	myrand::MyRand rng(25, 75);

	for (;;) {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			while (queue_.size() >= MaxSize && !quit) {
				std::cout << label_ << ": waiting for queue to have room..." << std::endl;
				queue_cond.wait(lock);
			}
			if (quit) return;

			std::cout << label_ << ": putting new item on queue" << std::endl;
			queue_.push(Object(rng()));
		}
		queue_cond.notify_one();

		if (quit) return;
		usleep(rng() * 10000);
	}
}

void Consumer::operator()() const {
	myrand::MyRand rng(35, 85);

	for (;;) {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			while (queue_.empty() && !quit) {
				std::cout << label_ << ": waiting for queue to have an item..." << std::endl;
				queue_cond.wait(lock);
			}
			if (quit) return;

			std::cout << label_ << ": getting next item from queue: ";
			Object item = queue_.front();
			queue_.pop();
			std::cout << item.v() << std::endl;
		}
		queue_cond.notify_one();

		if (quit) return;
		usleep(rng() * 10000);
	}
}

}

int main(int argc, char** argv) {
	std::vector<std::thread> threads;

	Producer p1("p1");
	Producer p2("p2");
	Consumer c1("c1");
	Consumer c2("c2");

	threads.emplace_back(p1);
	threads.emplace_back(p2);
	threads.emplace_back(c1);
	threads.emplace_back(c2);

	sleep(45);

	quit = true;
	queue_cond.notify_all();

	for (auto& t : threads) {
		t.join();
	}

	while (!queue_.empty()) {
		std::cout << queue_.front().v() << '\t';
		queue_.pop();
	}
	std::cout << std::endl;

	return 0;
}

