#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace patches
{
	namespace
	{
		utils::hook::detour wnd_proc_hook;

		struct event_t
		{
			std::uint32_t target_msg;
			std::uint32_t target_param;
			std::uint32_t msg;
			std::uint32_t param;
		};

		std::unordered_map<std::uint32_t, std::uint32_t> binds;		// native to custom
		std::vector<event_t> event_detours;

		LRESULT wnd_proc_stub(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
		{
			for (const auto& detour : event_detours)
			{
				if (msg == detour.target_msg && (detour.target_param == 0 || detour.target_param == w_param))
				{
					msg = detour.msg;
					w_param = detour.param;
				}
			}

			return wnd_proc_hook.invoke<LRESULT>(hwnd, msg, w_param, l_param);
		}

		SHORT get_async_get_state(const std::uint32_t key)
		{
			auto result = GetAsyncKeyState(key);

			const auto iter = binds.find(key);
			if (iter != binds.end())
			{
				result |= GetAsyncKeyState(iter->second);
			}

			return result;
		}

		void add_event_detour(const std::uint32_t target_msg, const std::uint32_t new_msg, const std::uint32_t target_param, std::uint32_t param)
		{
			event_t ev{};
			ev.target_msg = target_msg;
			ev.target_param = target_param;

			ev.msg = new_msg;
			ev.param = param;

			event_detours.emplace_back(ev);
		}

		void add_keybind(const std::uint32_t custom, const std::uint32_t native)
		{
			add_event_detour(WM_KEYUP, WM_KEYUP, custom, native);
			add_event_detour(WM_KEYDOWN, WM_KEYDOWN, custom, native);

			binds.insert(std::make_pair(native, custom));
		}

		void add_mouse_bind(const std::uint32_t custom, const std::uint32_t native, const bool up)
		{
			add_event_detour(custom, WM_KEYDOWN + up, 0, native);
		}
	}

	class component final : public component_interface
	{
	public:
		void on_startup() override
		{
			patch();
		}

		void patch()
		{
			add_keybind(VK_SPACE, VK_RETURN);
			add_keybind(VK_TAB, 'C');

			add_mouse_bind(WM_LBUTTONDOWN, VK_RETURN, 0);
			add_mouse_bind(WM_LBUTTONUP, VK_RETURN, 1);

			add_mouse_bind(WM_RBUTTONDOWN, 'X', 0);
			add_mouse_bind(WM_RBUTTONUP, 'X', 1);

			utils::hook::nop(0x1401AE4A2, 6);
			utils::hook::far_call<BASE_ADDRESS>(0x1401AE4A2, get_async_get_state);
			utils::hook::nop(0x1401AE4DC, 6);
			utils::hook::far_call<BASE_ADDRESS>(0x1401AE4DC, get_async_get_state);

			wnd_proc_hook.create(0x14004B280, wnd_proc_stub);
		}
	};
}

REGISTER_COMPONENT(patches::component)
