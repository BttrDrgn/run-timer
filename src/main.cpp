#include <sdk/SexySDK.hpp>
#include <callbacks/callbacks.hpp>

void main_loop();
void start_timer();
void end_timer();
std::string format_duration(std::chrono::milliseconds input);

std::chrono::steady_clock::time_point start_time;
std::chrono::steady_clock::time_point end_time;
std::chrono::steady_clock::duration elapsed;

std::chrono::steady_clock::time_point il_start_time;
std::chrono::steady_clock::time_point il_end_time;
std::chrono::steady_clock::duration il_elapsed;

bool running, il_running = false;
bool can_display_text = false;
std::string time_string, il_time_string;

void init()
{
	//Main loop
	callbacks::on(callbacks::type::main_loop, main_loop);

	//On adventure restart
	callbacks::on(callbacks::type::after_show_adventure_screen, start_timer);

	//On new save
	callbacks::on(callbacks::type::after_start_adventure_game, start_timer);

	//On finish
	callbacks::on(callbacks::type::just_beat_adventure_true, end_timer);

	//At some points during the game you cannot display text or else it will crash/have issues
	callbacks::on(callbacks::type::beginturn2, []()
	{
		if (running) can_display_text = true;
	});

	callbacks::on(callbacks::type::do_to_menu, []()
	{
		if (running)
		{
			running = false;
			can_display_text = false;
		}
	});

	callbacks::on(callbacks::type::do_options_dialog, []()
	{
		if (running) can_display_text = false;
	});

	callbacks::on(callbacks::type::finish_options_dialog, []()
	{
		if (running) can_display_text = true;
	});

	callbacks::on(callbacks::type::finish_init_level, []()
	{
		if (running)
		{
			il_start_time = std::chrono::steady_clock::now();
			il_running = true;
		}
	});

	callbacks::on(callbacks::type::do_level_done, []()
	{
		if (running) il_running = false;
	});
}

void main_loop()
{
	if (running)
	{
		end_time = std::chrono::steady_clock::now();
		elapsed = (end_time - start_time);

		if (il_running)
		{
			il_end_time = std::chrono::steady_clock::now();
			il_elapsed = (il_end_time - il_start_time);
		}

		time_string = format_duration(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed));
		il_time_string = format_duration(std::chrono::duration_cast<std::chrono::milliseconds>(il_elapsed));

		if (can_display_text)
		{
			Sexy::FloatingText_* time_text = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(
				Sexy::Format("Time: %s", time_string.c_str()),
				110.0f,
				30.0f,
				14
			);

			time_text->unk_1 = 1;

			Sexy::FloatingText_* il_time_text = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(
				Sexy::Format("Level: %s", il_time_string.c_str()),
				500.0f,
				30.0f,
				14
			);

			il_time_text->unk_1 = 1;
			il_time_text->color = 0x93E9BE;
		}
	}
}

void start_timer()
{
	if (running) return;
	start_time = std::chrono::steady_clock::now();
	running = true;
}

void end_timer()
{
	if (!running) return;
	running = false;
	il_running = false;
	std::printf("----- Run Finished -----\n");
	std::printf("Time: %s\n", time_string.c_str());
}

//https://stackoverflow.com/a/50727438
std::string format_duration(std::chrono::milliseconds ms)
{
	auto secs = std::chrono::duration_cast<std::chrono::seconds>(ms);
	ms -= std::chrono::duration_cast<std::chrono::milliseconds>(secs);
	auto mins = std::chrono::duration_cast<std::chrono::minutes>(secs);
	secs -= std::chrono::duration_cast<std::chrono::seconds>(mins);
	auto hour = std::chrono::duration_cast<std::chrono::hours>(mins);
	mins -= std::chrono::duration_cast<std::chrono::minutes>(hour);

	std::stringstream ss;

	std::string h = std::to_string((hour).count());
	std::string m = std::to_string((mins).count());
	std::string s = std::to_string((secs).count());
	std::string mi = std::to_string((ms).count());

	if (std::stoi(h) > 0)
	{
		ss << h << ':'
			<< std::setw(2) << std::setfill('0') << m << ':'
			<< std::setw(2) << std::setfill('0') << s << '.'
			<< std::setw(3) << std::setfill('0') << mi;
	}
	else
	{
		ss << std::setw(2) << std::setfill('0') << m << ':'
			<< std::setw(2) << std::setfill('0') << s << '.'
			<< std::setw(3) << std::setfill('0') << mi;
	}

	return ss.str();
}

DWORD WINAPI OnAttachImpl(LPVOID lpParameter)
{
	init();
	return 0;
}

DWORD WINAPI OnAttach(LPVOID lpParameter)
{
	__try
	{
		return OnAttachImpl(lpParameter);
	}
	__except (0)
	{
		FreeLibraryAndExitThread((HMODULE)lpParameter, 0xDECEA5ED);
	}

	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, OnAttach, hModule, 0, nullptr);
		return true;
	}

	return false;
}
