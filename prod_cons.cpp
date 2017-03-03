#include <iostream>
//#include <signal.h>
#include <queue>
#include <cstdlib>
#include <string>
using namespace std;

#include <boost/version.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
using namespace boost;

#include "myrand.h"
using namespace myrand;

namespace
{
class Object
{
public:
	explicit Object(int v=0) : v_(v) { }
	virtual ~Object() { }

	int v() const { return v_; }
private:
	int v_;
};

typedef queue<Object> MyList_t;
MyList_t l;
mutex list_mutex;
condition list_cond;
volatile bool quit;
const MyList_t::size_type MaxSize = 20;

class Producer
{
public:
	explicit Producer(const string& l) : label(l) { }
	virtual ~Producer() { }

	void operator()() const;
private:
	//Producer(const Producer&);
	//Producer& operator(const Producer&);

	string label;
};

class Consumer
{
public:
	explicit Consumer(const string& l) : label(l) { }
	virtual ~Consumer() { }

	void operator()() const;
private:
	string label;
};

void Producer::operator()() const
{
	MyRand mr(25,75);
	mutex::scoped_lock lk(list_mutex, defer_lock);
	for (;;)
	{
		lk.lock();
		if (l.size() >= MaxSize)
		{
			cout << label << ": waiting for list to have room..." << endl;
			do
			{
				list_cond.wait(lk);
				__sync_synchronize();
				if (quit) return;
			} while (l.size() >= MaxSize);
		}
		cout << label << ": putting new item on list" << endl;
		l.push(MyList_t::value_type(mr()));
		lk.unlock();
		list_cond.notify_one();
		usleep(mr()*10000);
		__sync_synchronize();
		if (quit) return;
	}
}

void Consumer::operator()() const
{
	MyRand mr(35,85);
	mutex::scoped_lock lk(list_mutex, defer_lock);
	for (;;)
	{
		lk.lock();
		if (l.empty())
		{
			cout << label << ": waiting for list to have an item..." << endl;
			do
			{
				list_cond.wait(lk);
				__sync_synchronize();
				if (quit) return;
			} while (l.empty());
		}
		cout << label << ": getting next item from list: ";
		MyList_t::value_type o = l.front();
		l.pop();
		cout << o.v() << endl;
		lk.unlock();
		list_cond.notify_one();
		usleep(mr()*10000);
		__sync_synchronize();
		if (quit) return;
	}
}

}

int main(int argc, char** argv)
{
	quit = false;
	__sync_synchronize();
	Producer p1("p1");
	Producer p2("p2");
	Consumer c1("c1");
	Consumer c2("c2");
	Consumer c3("c3");
	thread_group tgrp;
	tgrp.create_thread(p1);
	tgrp.create_thread(p2);
	tgrp.create_thread(c1);
	tgrp.create_thread(c2);
	//tgrp.create_thread(c3);

	//sigset_t mask;
	//sigemptyset(&mask);
	//sigsuspend(&mask);

	sleep(45);

	quit = true;
	__sync_synchronize();
	list_cond.notify_all();
	tgrp.join_all();

	MyList_t::value_type o;
	while (!l.empty())
	{
		o = l.front();
		cout << o.v() << '\t';
		l.pop();
	}
	cout << endl;

	return 0;
}

