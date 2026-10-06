#ifndef MYRAND_MYRAND_H__
#define MYRAND_MYRAND_H__

#include <random>

namespace myrand {

class MyRand {
public:
	explicit MyRand(int min, int max);
	~MyRand() = default;

	int generate();
	int operator()() { return generate(); }

	MyRand(const MyRand&) = delete;
	MyRand& operator=(const MyRand&) = delete;

private:
	std::mt19937 engine_;
	std::uniform_int_distribution<int> distribution_;
};

}

#endif

