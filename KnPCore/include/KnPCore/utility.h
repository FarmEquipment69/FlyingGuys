#pragma once
#include <string>

#define EXEC_FAILURE_CODE 1235678;
namespace KnP {
	namespace utility {

		typedef struct {
			std::wstring out;
			std::wstring err;
			uint64_t exit_code;
		} exec_info;

		void exec_command(const std::wstring& cmd, exec_info* info);
		std::wstring get_exe_dir();
	}
}