#include "data.h"
#include "generated/kdmapper.h"
#include "generated/knpdriver.h"
#include "memory.h"
#include "hook.h"

#include <zlib.h>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

namespace KnP {

	namespace data
	{
		knpdriver create_driver()
		{
			knpdriver result;
			result.data = (unsigned char*)malloc(::knpdriver::size * sizeof(unsigned char));
			assert(result.data != NULL);
			result.size = ::knpdriver::size;

			uncompress(result.data, (uLongf*)&result.size, (unsigned char*)::knpdriver::data, sizeof(::knpdriver::data));

			return result;
		}

		kdmapper create_mapper()
		{
			kdmapper result;
			result.data = (unsigned char*)malloc(::kdmapper::size * sizeof(unsigned char));
			assert(result.data != NULL);
			result.size = ::kdmapper::size;

			uncompress(result.data, (uLongf*)&result.size, (unsigned char*)::kdmapper::data, sizeof(::kdmapper::data));

			return result;
		}


		bool modify_driver_bytecode(knpdriver* driver, unsigned char* first, const unsigned char& size_f, unsigned char* last, const unsigned char& size_l)
		{
			if (size_f > 31 || size_l > 31)
			{
				return false;
			}

			unsigned char* ptr = memory::find_memory_pattern(driver->data, driver->size, "02 48 b8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
			assert(ptr != NULL);
			if (!ptr)
			{
				return false;
			}

			//memset(ptr, 0x00, sizeof(unsigned char) * 0x40);
			
			ptr[0] = size_f;
			memcpy(&ptr[1], first, size_f);
			ptr[32] = size_l;
			memcpy(&ptr[33], last, size_l);

			return true;
		}

		bool modify_driver_module_location(knpdriver* driver, const char* location)
		{
			assert(location != NULL);
			size_t size = strlen(location);
			assert(size <= 511);

			unsigned char* ptr = memory::find_memory_string(driver->data, driver->size, "\\SystemRoot\\System32\\drivers\\dxgkrnl.sys", 41);

			assert(ptr != NULL);

			if (!ptr)
			{
				return false;
			}

			memcpy(ptr, location, size+1);

			return true;
		}

		bool modify_driver_module_name(knpdriver* driver, const char* name)
		{
			assert(name != NULL);
			size_t size = strlen(name);
			assert(size <= 254);

			unsigned char* ptr = memory::find_memory_string(driver->data, driver->size, "NtDxgkGetTrackedWorkloadStatistics", 35);

			assert(ptr != NULL);

			if (!ptr)
			{
				return false;
			}

			memcpy(ptr, name, size+1);
			memcpy((void*)hook::module_name, name, size+1);

			return true;
		}

		bool modify_driver_module_lib(const char* name)
		{
			assert(name != NULL);
			size_t size = strlen(name);
			assert(size <= 254);
			memcpy((void*)hook::lib, name, size + 1);
			return true;
		}

		utility::exec_info finalize_data(knpdriver* driver, kdmapper* mapper)
		{
			const std::wstring kdmap_path = utility::get_exe_dir() + L"\\kdcompressed.exe";

			std::ofstream wf(kdmap_path, std::ios::out | std::ios::binary);
			wf.write((const char*)mapper->data, mapper->size);
			wf.close();
			free(mapper->data);
			mapper->size = 0;
			mapper->data = nullptr;

			const std::wstring driver_path = KnP::utility::get_exe_dir() + L"\\driver.sys";

			wf = std::ofstream(driver_path, std::ios::out | std::ios::binary);
			wf.write((const char*)driver->data, driver->size);
			wf.close();

			free(driver->data);
			driver->size = 0;
			driver->data = nullptr;

			std::wstring command = kdmap_path + L" " + driver_path;

			KnP::utility::exec_info info;

			KnP::utility::exec_command(command, &info);

			_wremove(driver_path.c_str());
			_wremove(kdmap_path.c_str());

			return info;
		}
	}
}