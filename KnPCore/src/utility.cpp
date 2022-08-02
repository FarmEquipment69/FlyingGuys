#include "utility.h"
#include <assert.h>
#include <windows.h>

namespace KnP {
	namespace utility {

		PROCESS_INFORMATION CreateChildProcess(const std::wstring& cmd, HANDLE _ERR_WR, HANDLE _OUT_WR, exec_info* info);
		void ReadFromPipe(HANDLE err_RD, HANDLE out_RD, exec_info* info);

		void exec_command(const std::wstring& cmd, exec_info* output)
		{
			HANDLE out_RD, out_WR, err_RD, err_WR;
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.bInheritHandle = TRUE;
			sa.lpSecurityDescriptor = NULL;

			// Create a pipe for the child process's STDERR. 
			if (!CreatePipe(&err_RD, &err_WR, &sa, 0)) {
				output->err = L"FAILED TO CREATE ERROR PIPE";
				output->exit_code = EXEC_FAILURE_CODE;
				return;
			}
			// Ensure the read handle to the pipe for STDERR is not inherited.
			if (!SetHandleInformation(err_RD, HANDLE_FLAG_INHERIT, 0)) {
				output->err = L"FAILED TO HANDLE ERROR PIPE INFORMATION";
				output->exit_code = EXEC_FAILURE_CODE;
			}
			// Create a pipe for the child process's STDOUT. 
			if (!CreatePipe(&out_RD, &out_WR, &sa, 0)) {
				output->err = L"FAILED TO CREATE OUT PIPE";
				output->exit_code = EXEC_FAILURE_CODE;
			}
			// Ensure the read handle to the pipe for STDOUT is not inherited
			if (!SetHandleInformation(out_RD, HANDLE_FLAG_INHERIT, 0)) {
				output->err = L"FAILED TO HANDLE OUT PIPE INFORMATION";
				output->exit_code = EXEC_FAILURE_CODE;
			}

			PROCESS_INFORMATION info = CreateChildProcess(cmd, err_WR, out_WR, output);

			ReadFromPipe(err_RD, out_RD, output);
		}

		PROCESS_INFORMATION CreateChildProcess(const std::wstring& cmd, HANDLE _ERR_WR, HANDLE _OUT_WR, exec_info* info) {
			PROCESS_INFORMATION piProcInfo;
			STARTUPINFOW siStartInfo;
			bool bSuccess = FALSE;

			// Set up members of the PROCESS_INFORMATION structure. 
			ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

			// Set up members of the STARTUPINFO structure. 
			// This structure specifies the STDERR and STDOUT handles for redirection.
			ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
			siStartInfo.cb = sizeof(STARTUPINFO);
			siStartInfo.hStdError = _ERR_WR;
			siStartInfo.hStdOutput = _OUT_WR;
			siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

			LPWSTR cmdLine = const_cast<wchar_t*>(cmd.c_str());

			//TODO: Look into ShellExecuteEx for elevated process
			bSuccess = CreateProcessW(NULL,
				cmdLine,     // command line 
				NULL,          // process security attributes 
				NULL,          // primary thread security attributes 
				TRUE,          // handles are inherited 
				0,             // creation flags 
				NULL,          // use parent's environment 
				NULL,          // use parent's current directory 
				&siStartInfo,  // STARTUPINFO pointer 
				&piProcInfo);  // receives PROCESS_INFORMATION

			for (int i = 0; i < 60; i++) {
				DWORD d = WaitForSingleObject(piProcInfo.hProcess, 500);
				//if (forceKillChild) {
					//GE_CORE_DEBUG("FORCE KILL OBJECT");
				//	break;
				//}
				if (d != WAIT_TIMEOUT) break;
			}

			GetExitCodeProcess(piProcInfo.hProcess, (LPDWORD)&info->exit_code);

			CloseHandle(_ERR_WR);
			CloseHandle(_OUT_WR);

			CloseHandle(piProcInfo.hProcess);
			CloseHandle(piProcInfo.hThread);
			// If an error occurs, exit the application. 
			if (!bSuccess) {

			}
			return piProcInfo;
		}

		void ReadFromPipe(HANDLE err_RD, HANDLE out_RD, exec_info* info) {
			DWORD dwRead;
			WCHAR* chBuf = (WCHAR*)calloc(20000, sizeof(WCHAR));
			assert(chBuf);
			bool bSuccess = FALSE;
			std::wstring out = L"", err = L"";
			for (;;) {
				bSuccess = ReadFile(out_RD, chBuf, 20000, &dwRead, NULL);
				if (!bSuccess || dwRead == 0) break;

				std::wstring s(chBuf, dwRead);
				out += s;
			}
			dwRead = 0;
			for (;;) {
				bSuccess = ReadFile(err_RD, chBuf, 20000, &dwRead, NULL);
				if (!bSuccess || dwRead == 0) break;

				std::wstring s(chBuf, dwRead);
				err += s;

			}

			CloseHandle(err_RD);
			CloseHandle(out_RD);
			free(chBuf);
			info->err = err;
			info->out = out;
		}

		std::wstring get_exe_dir()
		{
			wchar_t NPath[MAX_PATH];
			GetModuleFileNameW(NULL, NPath, MAX_PATH);
			std::wstring path(NPath);
			size_t slash = path.find_last_of(L"\\");
			return path.substr(0, slash);
		}
	}
}