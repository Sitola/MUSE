#ifndef MUSA_HPP
#define MUSA_HPP

#include "typedefs.hpp"

#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

// KERAT
#include <kerat/kerat.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/listener.hpp>
#include <kerat/tuio_messages.hpp>
#include <kerat/bundle.hpp>
#include <kerat/simple_client.hpp>

// SAGE
#include "sage_client.hpp"

#define MAX_TOUCHES 20

class musa : public libkerat::listener
{
	private:

		sage_client * sage;
		sem_t *semaphore;

		unsigned display_width;
		unsigned display_height;

		libkerat::client  * kerat_client;
		libkerat::simple_client * kerat_raw_client;
		libkerat::adaptor * kerat_multiplexer;
		bool kerat_multiplexing;
		uint16_t kerat_port;
		void get_kerat_default_config();
		bool running;

		int disp_Height;

		musa_touch touches[MAX_TOUCHES];
		int touches_num;
		void add_touch(musa_touch touch);
		void remove_touch(int id);
		musa_touch* get_touch(int id);
		void touch_process(musa_touch* touch);

		void msg_pointer(const libkerat::message::pointer * pointer);
		void msg_alive(const libkerat::message::alive * alive);

		void print();

	public:
		musa();
		virtual ~musa();
		void start();
		void stop();
		void notify(const libkerat::client * notifier);

		void add_semaphore(sem_t* sem)
		{
			semaphore = sem;
		}
		void add_sage(sage_client* sag)
		{
			sage = sag;
		}

};

#endif
