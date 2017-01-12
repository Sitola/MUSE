#include "sage_client.hpp"

sage_client::sage_client()
{
	windows_open = 0;
	running = false;
	char* sage_dir = getenv("SAGE_DIRECTORY");
	if (!sage_dir)
	{
		fprintf(stderr, "Nenasiel sa SAGE_DIRECTORY\n");
		return;
	}
	char conf_path[100];
	sprintf(conf_path, "%s/bin/%s", sage_dir, "fsManager.conf");
	// TODO: osetri navratove hodnoty mrkni throw exception
	sage.init(conf_path, (char *)"uiPort");
	sage.connect(NULL);
	sage.sendMessage(SAGE_UI_REG, (char *)"");
}

sage_client::~sage_client()
{

}

void sage_client::start_msg_recv()
{
	running = true;
	while (running)
	{
		sageMessage msg;
		sage.rcvMessageBlk(msg);
		int rc = msg.getCode();
		switch (rc)
		{
			case APP_INFO_RETURN:
				msg_app_info(msg);
				break;
			case UI_APP_SHUTDOWN:
				msg_app_shutdown(msg);
				break;
			case SAGE_DISPLAY_INFO:
				msg_display_info(msg);
				break;
			default:
				msg_undefined(msg);
				break;
		}
	}
}


void sage_client::msg_app_info(sageMessage msg)
{
	sage_window window;
	int ret_value = sscanf(
						(char*) msg.getData(),
						"%s %d %d %d %d %d %*d %d",
						window.appname,
						&window.appID,
						&window.bottom_left.x,
						&window.top_right.x,
						&window.bottom_left.y,
						&window.top_right.y,
						&window.zValue
					);

	if (ret_value != 7)
	{
		fprintf(stderr, "Error, got %d fields\n", ret_value);
		return;
	}

	create_window(window);

}

void sage_client::msg_app_shutdown(sageMessage msg)
{
	int appID = 0;
	int ret = sscanf((char*)msg.getData(), "%d", &appID);
	if (ret != 1)
	{
		fprintf(stderr, "Error, got %d fields\n", ret);
		return;
	}

	remove_window(appID);
}

void sage_client::msg_display_info(sageMessage msg)
{
	sscanf((char*)msg.getData(), "%*d %*d %*d %d %d", &disp_Width, &disp_Height);
}

void sage_client::msg_undefined(sageMessage msg)
{
	printf("Message %d:\n\"%s\"\n", msg.getCode(), (char*) msg.getData());
}

void sage_client::stop_msg_recv()
{
	running = false;
}

void sage_client::create_window(sage_window window)
{
	bool new_window = true;
	sem_wait(semaphore);
	for (int i = 0; i < windows_open; i++)
	{
		if (windows[i].appID == window.appID)
		{
			// updatnime info
			windows[i].bottom_left = window.bottom_left;
			windows[i].top_right = window.top_right;
			windows[i].zValue = window.zValue;
			new_window = false;
			break;
		}
	}
	if (new_window)   // nenasli sme nic, ideme pridat dalsie okno do pola
	{
		if (windows_open != WINDOWS_LIMIT)
		{
			windows[windows_open] = window;
			windows_open++;
		}
		else
		{
			fprintf(stderr, "Exceeded window limit\n");
		}
	}
	sem_post(semaphore);
}

void sage_client::remove_window(int appID)
{
	sem_wait(semaphore);
	for (int i = 0; i < windows_open; i++)
	{
		if (windows[i].appID == appID)
		{
			memmove(&windows[i], &windows[i + 1], sizeof(sage_window) * (windows_open - i));
			windows_open--;
		}
	}
	sem_post(semaphore);
}

sage_window sage_client::return_window(pixel position)
{
	int zValue = INT_MAX;
	int ret = -1;
	sem_wait(semaphore);
	for (int i = 0; i < windows_open; i++)
	{
		if (
			(windows[i].bottom_left.x < position.x) &&
			(windows[i].bottom_left.y < position.y) &&
			(windows[i].top_right.x > position.x) &&
			(windows[i].top_right.y > position.y) &&
			(windows[i].zValue <= zValue)
		)
		{
			zValue = windows[i].zValue;
			ret = i;
		}
	}
	sage_window window;
	window.appID = -1;
	if (ret != -1)
	{
		window = windows[ret];
	}
	sem_post(semaphore);
	return window;
}

void sage_client::sage_move(int appID, int dx, int dy)
{
	char arg[80];
	sprintf(arg, "%d %d %d", appID, dx, dy);
	sage.sendMessage(MOVE_WINDOW, arg);
}

void sage_client::sage_resize(int appid, pixel bottom_left, pixel top_right)
{
	char arg[80];
	sprintf(arg, "%i %i %i %i %i", appid, bottom_left.x, top_right.x, bottom_left.y, top_right.y);
	sage.sendMessage(RESIZE_WINDOW, arg);
}

void sage_client::sage_bg(int red, int green, int blue)
{
	char arg[80];
	sprintf(arg, "%d %d %d", red, green, blue);
	sage.sendMessage(SAGE_BG_COLOR, arg);
}

void sage_client::print()
{
	printf("Windows (%d):\n", windows_open);
	for (int i = 0; i < windows_open; i++)
	{
		printf("\t%d: {\n", i);
		printf("\t\tapp: %d (%s) \n", windows[i].appID, windows[i].appname);
		printf("\t\tpos: l:%d r:%d b:%d t:%d \n", windows[i].bottom_left.x, windows[i].top_right.x, windows[i].bottom_left.y, windows[i].top_right.y);
		printf("\t\tzval: %d \n", windows[i].zValue);
		printf("\t}\n");
	}
	printf("\n");
}
