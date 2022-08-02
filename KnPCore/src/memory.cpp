#include "memory.h"
#include "hook.h"

#include <string>
#include <assert.h>
#include <vector>
#include <memory>

namespace KnP {

	namespace memory
	{
		using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;

		uint32_t get_process_id(std::wstring process_name)
		{
			PROCESSENTRY32W processEntry;

			const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | 0x40000000, NULL));

			if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
			{
				return NULL;
			}

			processEntry.dwSize = sizeof(PROCESSENTRY32W);

			while (Process32NextW(snapshot_handle.get(), &processEntry) == TRUE)
			{
				if (process_name.compare(processEntry.szExeFile) == NULL)
				{
					return processEntry.th32ProcessID;
				}
			}

			return NULL;
		}

		unsigned char* find_memory_pattern(unsigned char* src, size_t size, const char* pattern)
		{
			std::vector<uint16_t> data;
			std::string strMask(pattern);
			
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

			for (size_t i = 0; i < size; i++)
			{
				if (i + data.size() > size)
				{
					break;
				}

				bool found = true;

				for (size_t j = 0; j < data.size(); j++)
				{
					if (data[j] > 0xff)
					{
						continue;
					}

					if (src[i + j] != static_cast<byte>(data[j]))
					{
						found = false;
						break;
					}
				}

				if (found)
				{
					return &src[i];
				}
			}

			return NULL;
		}

		unsigned char* find_memory_string(unsigned char* src, size_t size, const char* str, size_t strSize)
		{
			size_t sz = strSize;

			for (size_t i = 0; i < size; i++)
			{
				if (i + sz > size)
				{
					break;
				}

				bool found = true;

				for (size_t j = 0; j < sz; j++)
				{
				
					if (src[i + j] != static_cast<byte>(str[j]))
					{
						found = false;
						break;
					}
				}

				if (found)
				{
					return &src[i];
				}
			}

			return NULL;
		}

	}
}