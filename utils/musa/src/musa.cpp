#include "musa.hpp"

bool running;

static void handle_kill_signal(int ev)
{
	running = false; // this variable is checked in the main event loop
}

static void register_signal_handlers()
{
	signal(SIGTERM, handle_kill_signal);
	// NOTE: SIGKILL is still uncaughtable
	//signal(SIGKILL, handle_kill_signal); // can't do anything about it, so at least correctly close
	signal(SIGABRT, handle_kill_signal);
	signal(SIGINT, handle_kill_signal);
	signal(SIGHUP, handle_kill_signal);
}

void * kerat_thread(void* arg)
{
	musa * mu = (musa*)arg;
	mu->start();
	return 0;
}

void * sage_thread(void* arg)
{
	sage_client * sage = (sage_client*)arg;
	sage->start_msg_recv();
	return 0;
}

int main(int argc, char* argv[])
{
	register_signal_handlers();

	musa mu;
	sage_client sage;
	sem_t semaphore;
	pthread_t kerat_t, sage_t;

	sem_init(&semaphore, 0, 10);
	mu.add_semaphore(&semaphore);
	sage.add_semaphore(&semaphore);
	mu.add_sage(&sage);
	pthread_create(&kerat_t, NULL, kerat_thread, &mu);
	pthread_create(&sage_t, NULL, sage_thread, &sage);

	running = true;
	while (running);

	mu.stop();
	sage.stop_msg_recv();

	sem_destroy(&semaphore);

	return 0;
}





