#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace patches
{
	namespace
	{
		utils::hook::detour wnd_proc_hook;

		std::unordered_map<std::uint32_t, std::uint32_t> binds;		// native to custom
		std::unordered_map<std::uint32_t, std::uint32_t> binds_rev; // custom to native

		LRESULT wnd_proc_stub(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
		{
			switch (msg)
			{
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				const auto iter = binds_rev.find(static_cast<std::uint32_t>(w_param));
				if (iter != binds_rev.end())
				{
					w_param = iter->second;
				}
			}
			break;
			case WM_LBUTTONDOWN:
			{
				msg = WM_KEYDOWN;
				w_param = VK_RETURN;
			}
			break;
			case WM_LBUTTONUP:
			{
				msg = WM_KEYUP;
				w_param = VK_RETURN;
			}
			break;
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

		void add_keybind(const std::uint32_t custom, const std::uint32_t native)
		{
			binds.insert(std::make_pair(native, custom));
			binds_rev.insert(std::make_pair(custom, native));
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

			utils::hook::nop(0x1401AE4A2, 6);
			utils::hook::far_call<BASE_ADDRESS>(0x1401AE4A2, get_async_get_state);
			utils::hook::nop(0x1401AE4DC, 6);
			utils::hook::far_call<BASE_ADDRESS>(0x1401AE4DC, get_async_get_state);

			wnd_proc_hook.create(0x14004B280, wnd_proc_stub);
		}
	};
}

REGISTER_COMPONENT(patches::component)
