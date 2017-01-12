#ifndef SAGE_CLIENT_HPP
#define SAGE_CLIENT_HPP

#include "typedefs.hpp"
#include <fsClient.h>

#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define WINDOWS_LIMIT 50

class sage_client
{

	private:
		fsClient sage; //Nasa hviezda

		sage_window windows[WINDOWS_LIMIT];
		int windows_open; //Pocet otvorenych okien
		bool running;
		int disp_Width;
		int disp_Height;

		void msg_app_info(sageMessage msg);
		void msg_app_shutdown(sageMessage msg);
		void msg_undefined(sageMessage msg);
		void msg_display_info(sageMessage msg);

		void create_window(sage_window window);
		void remove_window(int appID);

		sem_t * semaphore;

	public:
		sage_client();
		virtual ~sage_client();

		void add_semaphore(sem_t* sem)
		{
			semaphore = sem;
		}

		void start_msg_recv();
		void stop_msg_recv();

		int get_disp_width()
		{
			return disp_Width;
		}
		int get_disp_height()
		{
			return disp_Height;
		}

		void sage_move(int appID, int dx, int dy);
		void sage_resize(int appID, pixel bottom_left, pixel top_right);
		void sage_bg(int red, int green, int blue);

		sage_window return_window(pixel position);
		void print();
};

#endif
