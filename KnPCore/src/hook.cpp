#include "hook.h"
#include <vector>
#include <string>
#include <assert.h>

namespace KnP {

	namespace hook
	{
		char lib[255] = "win32u.dll";
		char module_name[255] = "NtDxgkGetTrackedWorkloadStatistics";

		int wingdi::draw_box(int x, int y, int w, int h, int t, int r, int g, int b)
		{
			KnP::memory::MEMORY instruction;
			instruction.r = r;
			instruction.g = g;
			instruction.b = b;

			instruction.x = x;
			instruction.y = y;
			instruction.w = w;
			instruction.h = h;
			instruction.t = t;

			instruction.action = ACTION_DRAWBOX;

			call_hook(&instruction);

			return 0;
		}

		int wingdi::draw_text(const char* str, int x, int y, int w, int h)
		{
			KnP::memory::MEMORY instruction;

			instruction.x = x;
			instruction.y = y;
			instruction.w = w;
			instruction.h = h;

			instruction.text = (char*)str;

			instruction.action = ACTION_DRAWTEXT;
			instruction.output = malloc(sizeof(int));
			call_hook(&instruction);
			assert(instruction.output != NULL);
			int result = *(int*)instruction.output;
			free(instruction.output);

			return result;
		}

		std::string memory::read_memory_string(UINT_PTR readAddr, ULONG pid)
		{
			std::string result = "";
			char ch = -1;
			size_t counter = 0;

			while (ch != 0 && counter < 1024)
			{
				ch = read_memory<char>(readAddr + counter, pid);
				if (ch != 0) result += ch;
				counter += sizeof(char);
			}

			return result;
		}

		KnP::memory::ModuleInfo memory::get_module_information(const char* module_name, ULONG pid)
		{
			KnP::memory::MEMORY instructions = { 0 };
			instructions.pid = pid;
			instructions.action = ACTION_REQBASE;
			instructions.module_name = module_name;
			hook::call_hook(&instructions);

			return instructions.module_info;
		}

		std::wstring memory::read_memory_wstring(UINT_PTR readAddr, ULONG pid)
		{
			std::wstring result = L"";
			wchar_t ch = -1;
			size_t counter = 0;

			while (ch != 0 && counter < 1024)
			{
				ch = read_memory<wchar_t>(readAddr + counter, pid);
				if (ch != 0) result += ch;
				counter += sizeof(wchar_t);
			}

			return result;
		}

		void memory::read_memory(UINT_PTR readAddr, void* output, size_t size, ULONG pid)
		{
			KnP::memory::MEMORY instructions;
			instructions.pid = pid;
			instructions.size = size;

			instructions.addr = readAddr;
			instructions.action = ACTION_READ_KERNEL;
			instructions.output = output;
			call_hook(&instructions);
		}

		void memory::write_memory(UINT_PTR writeAddr, void* input, size_t size, ULONG pid)
		{
			assert(size > 0);
			if (size <= 0) return;
			void* buffer = malloc(size);	

			assert(buffer != NULL);
			if (!buffer) return;

			memcpy(buffer, input, size);

			KnP::memory::MEMORY instructions;
			instructions.pid = pid;
			instructions.action = ACTION_WRITE_KERNEL;

			instructions.addr = writeAddr;
			instructions.buffer_addr = buffer;
			instructions.size = size;
			call_hook(&instructions);

			free(buffer);
		}

		UINT_PTR memory::scan_signature(UINT_PTR readAddr, ULONG size, const char* mask, ULONG pid)
		{
			std::vector<uint16_t> data;
			std::string strMask(mask);

			for (size_t i = 0; i < strMask.size(); i++)
			{
				if (strMask[i] == ' ')
				{
					continue;
				}

				if (i + 1 > strMask.size())
				{
					return NULL;
				}

				std::string s = strMask.substr(i, 2);

				if (s.find("?") != std::string::npos)
				{
					data.push_back(0x100);
					i++;
					continue;
				}

				data.push_back(std::stoi(s, nullptr, 16));
				i++;
			}

			byte* buffer = (byte*)malloc(sizeof(byte) * data.size());
			assert(buffer != NULL);

			for (size_t i = 0; i < size; i++)
			{
				if (i + data.size() > size)
				{
					break;
				}

				memory::read_memory(readAddr + i, buffer, data.size() * sizeof(byte), pid);
				bool found = true;

				for (size_t j = 0; j < data.size(); j++)
				{
					if (data[j] > 0xff)
					{
						continue;
					}

					if (buffer[i + j] != static_cast<byte>(data[j]))
					{
						found = false;
						break;
					}
				}

				if (found)
				{
					free(buffer);
					return i;
				}
			}

			free(buffer);
			return NULL;
		}

	}
}