#include "myrand.h"
#include <stdexcept>

namespace myrand {

MyRand::MyRand(int min, int max)
	: engine_(std::random_device{}()),
	  distribution_(min, max) {
	if (max < min) {
		throw std::range_error("max < min");
	}
}

int MyRand::generate() {
	return distribution_(engine_);
}

}

