#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/binary_resource.hpp>
#include <utils/nt.hpp>
#include <utils/string.hpp>

namespace
{
	utils::nt::library load_dinput8_library()
	{
		char dir[MAX_PATH]{};
		GetSystemDirectoryA(dir, sizeof(dir));
		return utils::nt::library::load(dir + "/dinput8.dll"s);
	}

	utils::nt::library& get_dinput8_library()
	{
		static auto lib = load_dinput8_library();
		return lib;
	}

	FARPROC WINAPI get_proc_address(const HMODULE module, const LPCSTR lp_proc_name)
	{
		if (lp_proc_name == "DirectInput8Create"s)
		{
			return get_dinput8_library().get_proc<FARPROC>(lp_proc_name);
		}

		return GetProcAddress(module, lp_proc_name);
	}

	void get_proc_address_stub(utils::hook::assembler& a)
	{
		a.pushad64();
		a.call_aligned(get_proc_address);
		a.popad64();

		a.mov(rcx, qword_ptr(0x140675970));
		a.jmp(0x1400040E0);
	}
}

BOOL APIENTRY DllMain(HMODULE module, DWORD ul_reason_for_call, LPVOID /*reserved*/)
{
	utils::hook::nop(0x1400040D3, 6);
	utils::hook::far_call<BASE_ADDRESS>(0x1400040D3, get_proc_address);

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		OutputDebugString("startup plugin\n");
		printf("here\n");
		component_loader::on_startup();
		utils::nt::library::set_current_handle(module);
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		OutputDebugString("shutdown\n");

		component_loader::on_shutdown();
	}

	return TRUE;
}
