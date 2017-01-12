#include "musa.hpp"

musa::musa()
{
	kerat_multiplexing	= true;
	kerat_port			= 3333;
	kerat_raw_client	= new libkerat::simple_client(kerat_port);
	kerat_client		= kerat_raw_client;
	kerat_multiplexer	= NULL;

	if (kerat_multiplexing)
	{
		kerat_multiplexer = new libkerat::adaptors::multiplexing_adaptor;
		kerat_client->add_listener(kerat_multiplexer);
		kerat_client = kerat_multiplexer;
	}

	kerat_client->add_listener(this);

	touches_num = 0;
}

musa::~musa()
{
	delete kerat_raw_client;
	delete kerat_multiplexer;
}

void musa::add_touch(musa_touch touch)
{
	if (touches_num != MAX_TOUCHES)
	{
		printf("adding touch\n");
		touches[touches_num] = touch;
		touches_num++;
	}
	else
	{
		fprintf(stderr, "Exceeded touch limit\n");
	}

}

void musa::remove_touch(int i)
{
	memmove(&touches[i], &touches[i + 1], sizeof(musa_touch) * (touches_num - i));
	touches_num--;
}

musa_touch* musa::get_touch(int id)
{
	for (int i = 0; i < touches_num; i++)
	{
		if (touches[i].id == id)
		{
			return &touches[i];
		}
	}
	return NULL;
}

void musa::start(void)
{
	running = true;
	disp_Height = sage->get_disp_height();
	while (running)
	{
		kerat_client->load();
	}
}

void musa::stop()
{
	running = false;
}

void musa::notify(const libkerat::client * notifier)
{
	libkerat::bundle_stack stack = notifier->get_stack();
	while (stack.get_length() > 0)
	{
		libkerat::bundle_handle f = stack.get_update();

		typedef libkerat::bundle_handle::const_iterator iterator;
		for (iterator i = f.begin(); i != f.end(); i++)
		{
			const libkerat::message::pointer * pointer = dynamic_cast<const libkerat::message::pointer *>(*i);
			if (pointer != NULL)
			{
				msg_pointer(pointer);
			}

			const libkerat::message::alive * alive = dynamic_cast<const libkerat::message::alive  *>(*i);
			if (alive != NULL)
			{
				msg_alive(alive);
			}

		}
		sage->print();
		print();

	}
}
void musa::msg_pointer(const libkerat::message::pointer * pointer)
{
	libkerat::coord_t sx = pointer->get_x();
	libkerat::coord_t sy = pointer->get_y();
	libkerat::user_id_t id = pointer->get_session_id();
	pixel position =
	{
		(int)sx,
		(int)(disp_Height - sy)
	};
	musa_touch* touch = get_touch(id);
	if (touch)
	{
		if ((touch->start.x == -1) && (touch->start.y == -1))
		{
			sage_window window = sage->return_window(position);
			if (window.appID != -1)
			{
				touch->appID = window.appID;
			}
			else
			{
				touch->appID = -1;
			}
			touch->start = position;
		}
		else
		{
			touch->end = position;
		}
	}
	else
	{
		musa_touch new_touch;
		new_touch.start = position;
		new_touch.id = id;
		sage_window window = sage->return_window(position);
		if (window.appID != -1)
		{
			new_touch.appID = window.appID;
		}
		else
		{
			new_touch.appID = -1;
		}
		add_touch(new_touch);
	}

}

void musa::msg_alive(const libkerat::message::alive * msg_alv)
{
	libkerat::message::alive::alive_ids alives = msg_alv->get_alives();
	for (int i = 0; i < touches_num; i++)
	{
		touches[i].alive = false;
	}
	for (libkerat::message::alive::alive_ids::const_iterator alive = alives.begin(); alive != alives.end(); alive++)
	{
		musa_touch * touch = get_touch(*alive);
		if (touch)
		{
			touch->alive = true;
		}
		else
		{
			musa_touch new_touch;
			new_touch.start.x = -1;
			new_touch.start.y = -1;
			new_touch.id = *alive;
			new_touch.appID = -1;
			add_touch(new_touch);
		}
	}
	for (int i = 0; i < touches_num; i++)
	{
		if (!touches[i].alive)
		{
			touch_process(&touches[i]);
			remove_touch(i);
		}
	}
}

void musa::touch_process(musa_touch* touch)
{
	if (touch->appID != -1)
	{
		printf("ID: %d, Moving %d, dx: %d px, dy: %d px\n", touch->id, touch->appID, (touch->end.x - touch->start.x), (touch->end.y - touch->start.y));
		sage->sage_move(touch->appID, (touch->end.x - touch->start.x), (touch->end.y - touch->start.y));
	}
}

void musa::print()
{
	printf("Touches (%d):\n", touches_num);
	for (int i = 0; i < touches_num; i++)
	{
		printf("\t%d: {\n", i);
		printf("\t\tid: %d, %s\n", touches[i].id, (touches[i].alive) ? "alive" : "-");
		printf("\t\tappid: %d \n", touches[i].appID);
		printf("\t\ttouch_start: [%d,%d] \n", touches[i].start.x, touches[i].start.y);
		printf("\t\ttouch_end: [%d,%d] \n", touches[i].end.x, touches[i].end.y);
		printf("\t}\n");
	}
	printf("\n");
}
