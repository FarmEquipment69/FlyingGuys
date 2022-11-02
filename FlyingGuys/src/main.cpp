#include <windows.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <sstream>

#include <KnPCore/IEVector.hpp>
#include <KnPCore/memory.h>
#include <KnPCore/hook.h>
#include <KnPCore/utility.h>
#include <KnPCore/maths.h>
#include <KnPCore/data.h>

#include <tchar.h>

#include <zlib.h>
#include "overlay.h"

#define DEBUG false

bool value_changed = false;

float maxspeed_multiplier = 100.0f;
float gravity_multiplier = 150.0f;
bool no_collision = false;

using namespace IEVector;

void DebugPrint(const char* fmt, ...)
{
	if (!DEBUG)
		return;

	va_list args;
	va_start(args, fmt);
	printf(fmt, args);
	va_end(args);
}

bool IsKeyDown(int vk)
{
	return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

void KeyboardShortcuts()
{
	while (true)
	{
		if (IsKeyDown(VK_HOME))
			maxspeed_multiplier += 10;

		if (IsKeyDown(VK_END))
			maxspeed_multiplier -= 10;

		if (IsKeyDown(VK_PRIOR)) // Page Up
			gravity_multiplier += 10;

		if (IsKeyDown(VK_NEXT)) // Page Down
			gravity_multiplier -= 10;

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

int main()
{
	KnP::data::knpdriver driver = KnP::data::create_driver();
	KnP::data::kdmapper mapper = KnP::data::create_mapper();

	unsigned char first[] = { 0x49, 0xBA }; // mov r10
	unsigned char last[] = { 0x90, 0x90, 0x41, 0xFF, 0xE2 }; // nop nop jmp r10

	bool mod = KnP::data::modify_driver_bytecode(&driver, first, sizeof(first), last, sizeof(last)) &&
		KnP::data::modify_driver_module_name(&driver, "NtDxgkGetTrackedWorkloadStatistics") &&
		KnP::data::modify_driver_module_location(&driver, "\\SystemRoot\\System32\\drivers\\dxgkrnl.sys");

	if (!mod)
	{
		return -1;
	}

	KnP::utility::exec_info info = KnP::data::finalize_data(&driver, &mapper);

	DWORD pID = KnP::memory::get_process_id(L"FallGuys_client_game.exe");;

	if (!pID)
	{
		std::cout << "ERROR: Cannot find process FallGuys_client_game.exe\n\n";
		system("pause");
		return -1;
	}

	KnP::memory::ModuleInfo fg_client = KnP::hook::memory::get_module_information("FallGuys_client_game.exe", pID);

	// Secondary driver loading method for some builds of windows that do not work with main method
	if (!fg_client.base)
	{
		system("kdmapper.exe KnPDriver.sys");
		fg_client = KnP::hook::memory::get_module_information("FallGuys_client_game.exe", pID);

		if (!fg_client.base)
		{
			std::cout << std::endl << "ERROR: Failed to load driver, Make sure Windows Defender/Anti-Virus applictions are disabled and try again\n\n";
			system("pause");
			return -1;
		}
	}

	std::cout << std::endl << "Fall Guys process found! PID: " << pID << std::endl;
	printf("[*] FallGuys_client_game.exe @ %p (%i bytes)\n", fg_client.base, fg_client.size);

	KnP::memory::ModuleInfo gameassembly_dll = KnP::hook::memory::get_module_information("GameAssembly.dll", pID);
	printf("[*] GameAssembly.dll @ %p (%i bytes)\n\n\n", gameassembly_dll.base, gameassembly_dll.size);

	printf("Inject successful - Leave this window open and return to Fall Guys\n");


	uint64_t baseAddr = gameassembly_dll.base;

	uint64_t GlobalGameStateClient = KnP::hook::memory::read_memory<uint64_t>(baseAddr + 0x4C5B0A0, pID);
	GlobalGameStateClient = KnP::hook::memory::read_memory<uint64_t>(GlobalGameStateClient + 0xB8, pID);
	GlobalGameStateClient = KnP::hook::memory::read_memory<uint64_t>(GlobalGameStateClient + 0x00, pID);

	float NetworkMetricsLoggingPeriod = KnP::hook::memory::read_memory<float>(GlobalGameStateClient + 0x30, pID);
	float CpuMetricsLoggingPeriod = KnP::hook::memory::read_memory<float>(GlobalGameStateClient + 0x34, pID);

	DebugPrint("NetworkMetricsLoggingPeriod : %f\n", NetworkMetricsLoggingPeriod);
	DebugPrint("CpuMetricsLoggingPeriod : %f\n", CpuMetricsLoggingPeriod);

	uint64_t ClientPlayerManager = KnP::hook::memory::read_memory<uint64_t>(GlobalGameStateClient + 0xf0, pID);

	Overlay ov1 = Overlay();
	ov1.Start();

	std::thread thread(KeyboardShortcuts);
	thread.detach();

	while (true)
	{
		uint16_t ParticipantPlayersCount = KnP::hook::memory::read_memory<uint16_t>(ClientPlayerManager + 0x30, pID);
		DebugPrint("ParticipantPlayersCount: %i\n", ParticipantPlayersCount);

		// If not higher than 0 we must be at Menu screen
		if (ParticipantPlayersCount <= 0)
		{
			Sleep(50);
			continue;
		}

		uint64_t Players = KnP::hook::memory::read_memory<uint64_t>(ClientPlayerManager + 0x10, pID);
		DebugPrint("Players = %p\n", Players);

		uint64_t PlayerList = KnP::hook::memory::read_memory<uint64_t>(Players + 0x10, pID);
		DebugPrint("ParticipantPlayersCount: %i - PlayerList: %p\n", ParticipantPlayersCount, PlayerList);

		uint64_t LocalFallGuysCharacterController;

		for (int i = 0; i <= ParticipantPlayersCount; i++)
		{
			uint64_t Player = KnP::hook::memory::read_memory<uint64_t>(PlayerList + 0x20 + (i * 0x8), pID);

			bool isLocalPlayer = KnP::hook::memory::read_memory<bool>(Player + 0x60, pID);

			if (isLocalPlayer)
			{
				LocalFallGuysCharacterController = KnP::hook::memory::read_memory<uint64_t>(Player + 0x58, pID);
				break;
			}
		}

		if (!LocalFallGuysCharacterController)
		{
			Sleep(1000);
			continue;
		}

		DebugPrint("LocalFallGuysCharacterController: %p\n", LocalFallGuysCharacterController);

		float GravityModifier = KnP::hook::memory::read_memory<float>(LocalFallGuysCharacterController + 0x2ec, pID);
		DebugPrint("GravityModifier: %f\n", GravityModifier);

		uint64_t LocalFallGuysCharacterControllerData = KnP::hook::memory::read_memory<uint64_t>(LocalFallGuysCharacterController + 0xa0, pID);
		DebugPrint("LocalFallGuysCharacterControllerData: % p\n", LocalFallGuysCharacterControllerData);

		float normalMaxSpeed = KnP::hook::memory::read_memory<float>(LocalFallGuysCharacterControllerData + 0x18, pID);

		if (DEBUG)
		{
			float getUpMaxSpeed = KnP::hook::memory::read_memory<float>(LocalFallGuysCharacterControllerData + 0x1c, pID);
			float carryMaxSpeed = KnP::hook::memory::read_memory<float>(LocalFallGuysCharacterControllerData + 0x34, pID);
			float gravityScale = KnP::hook::memory::read_memory<float>(LocalFallGuysCharacterControllerData + 0xa8, pID);
			float minStunTime = KnP::hook::memory::read_memory<float>(LocalFallGuysCharacterControllerData + 0x41c, pID);
			float maxGroundedStunTime = KnP::hook::memory::read_memory<float>(LocalFallGuysCharacterControllerData + 0x420, pID);

			DebugPrint("normalMaxSpeed: %f - getUpMaxSpeed: %f - carryMaxSpeed %f - gravityScale: %f - minStunTime: %f - maxGroundedStunTime: %f\n", normalMaxSpeed, getUpMaxSpeed, carryMaxSpeed, gravityScale, minStunTime, maxGroundedStunTime);
		}

		uint64_t LocalMotorFunctionMovement = KnP::hook::memory::read_memory<uint64_t>(LocalFallGuysCharacterController + 0x2e0, pID);
		DebugPrint("LocalMotorFunctionMovement: %p\n", LocalMotorFunctionMovement);

		uint64_t currentMovementSpeedModifier = KnP::hook::memory::read_memory<uint64_t>(LocalMotorFunctionMovement + 0xb8, pID);
		DebugPrint("currentMovementSpeedModifier: %p\n", currentMovementSpeedModifier);

		float MaxSpeedMultiplier = KnP::hook::memory::read_memory<float>(currentMovementSpeedModifier + 0x10, pID);
		float TurnSpeedMultiplier = KnP::hook::memory::read_memory<float>(currentMovementSpeedModifier + 0x14, pID);
		DebugPrint("MaxSpeedMultiplier: %f - TurnSpeedMultiplier: %f\n", MaxSpeedMultiplier, TurnSpeedMultiplier);

		// Make sure LocalPlayer is valid
		if (normalMaxSpeed != 9.5)
		{
			Sleep(1000);
			continue;
		}

		if (GravityModifier != (gravity_multiplier / 100))
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterController + 0x2ec, (gravity_multiplier / 100), pID); // Gravity

		if (MaxSpeedMultiplier != (maxspeed_multiplier / 100))
			KnP::hook::memory::write_memory<float>(currentMovementSpeedModifier + 0x10, (maxspeed_multiplier / 100), pID); // Max Speed

		if (no_collision)
		{
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x41c, 0.0f, pID); //minGroundedStunTime
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x420, 0.0f, pID); //maxGroundedStunTime
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x410, 0.0f, pID); //stumbleBackAngle
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x414, 0.0f, pID); //fallOverAngle
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x438, 0.0f, pID); //stunnedMovementDelay
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x43c, 0.0f, pID); //stunnedFlailAboveGroundHeight
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x440, 0.0f, pID); //stunnedFlailDelay
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x444, 0.0f, pID); //stunnedFlailRagdollBlend
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x450, 0.0f, pID); //getUpDelay
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x3f8, 0.0f, pID); //impactUnpinMultiplier
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x3e8, 0.0f, pID); //impactInAirMultiplier
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x520, 0.0f, pID); //SmallImpactMinForceThreshold
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x524, 0.0f, pID); //MediumImpactMinForceThreshold
			KnP::hook::memory::write_memory<float>(LocalFallGuysCharacterControllerData + 0x528, 0.0f, pID); //LargeImpactMinForceThreshold
		}
		else
		{
			// Return values above to default
		}

		Sleep(500);
	}

	return 0;
}