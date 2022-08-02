#pragma once

#include "utility.h"

namespace KnP {

	namespace data
	{
		typedef struct {
			unsigned char* data;
			size_t size;
		} kdmapper, knpdriver;

		knpdriver create_driver();
		kdmapper create_mapper();

		bool modify_driver_bytecode(knpdriver* driver, unsigned char* first, const unsigned char& size_f, unsigned char* last, const unsigned char& size_l);
		bool modify_driver_module_location(knpdriver* driver, const char* location);
		bool modify_driver_module_name(knpdriver* driver, const char* name);
		bool modify_driver_module_lib(const char* name);


		utility::exec_info finalize_data(knpdriver* driver, kdmapper* mapper);
	}

}