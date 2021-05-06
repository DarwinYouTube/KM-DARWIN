#include "main.h"
#include "offsets.h"

int main() {
	DarwinTap Driver("\\\\.\\kmdarwin");

	ProcessId = Driver.GetPid();
	ClientModule = Driver.GetClientModule();

	cout << "\n[>] Process ID: " << ProcessId << endl;
	cout << "[>] ClientModule: " << uppercase << hex << ClientModule << endl;

	while (true) {
		// Get LocalPlayer and Enemy
		DWORD LocalPlayer = Driver.ReadVirtualMemory<DWORD>(ProcessId, ClientModule + LocalPlayer_Address, sizeof(ULONG));
		// Get LocalPlayer Value
		DWORD MyTeam = Driver.ReadVirtualMemory<DWORD>(ProcessId, LocalPlayer + iTeamNum, sizeof(ULONG));

		//Color Fix
		float colorful = 1.0f;
		float colordow = 0.f;
		ULONG colorfull = *((unsigned long*)&colorful);
		ULONG colordown = *((unsigned long*)&colordow);

		// Get Other Value with ClientModule
		DWORD GlowObjectManager = Driver.ReadVirtualMemory<DWORD>(ProcessId, ClientModule + GlowObjectManager_Address, sizeof(ULONG));
		for (int x = 0; x < 32; x++) {

			DWORD Enemy = Driver.ReadVirtualMemory<DWORD>(ProcessId, ClientModule + EntityList_Address + x * 0x10, sizeof(ULONG));

			// Get Enemy Value
			DWORD EnemyTeam = Driver.ReadVirtualMemory<DWORD>(ProcessId, Enemy + iTeamNum, sizeof(ULONG));
			DWORD EnemyGlowIndex = Driver.ReadVirtualMemory<DWORD>(ProcessId, Enemy + glowIndex_Address, sizeof(ULONG));
			DWORD EnemyDormant = Driver.ReadVirtualMemory<DWORD>(ProcessId, Enemy + Dormant_Address, sizeof(ULONG));

			// Anti-Flash
			DWORD FlashDuration = Driver.ReadVirtualMemory<DWORD>(ProcessId, LocalPlayer + FlashDuration_Address, sizeof(ULONG));
			if (FlashDuration > 0)
				Driver.WriteVirtualMemory(ProcessId, LocalPlayer + FlashDuration_Address, 0, sizeof(ULONG));

			// Radar Hack
			if (EnemyTeam != MyTeam && !EnemyDormant)
				Driver.WriteVirtualMemory(ProcessId, Enemy + Spotted_Address, 1, sizeof(ULONG));

			// Glow
			if (EnemyTeam != MyTeam) {

				Driver.WriteVirtualMemory(ProcessId, GlowObjectManager + EnemyGlowIndex * 0x38 + 0x4, colorfull, sizeof(ULONG));
				Driver.WriteVirtualMemory(ProcessId, GlowObjectManager + EnemyGlowIndex * 0x38 + 0x8, colorfull, sizeof(ULONG));
				Driver.WriteVirtualMemory(ProcessId, GlowObjectManager + EnemyGlowIndex * 0x38 + 0xC, colorfull, sizeof(ULONG));
				Driver.WriteVirtualMemory(ProcessId, GlowObjectManager + EnemyGlowIndex * 0x38 + 0x10, colorfull, sizeof(ULONG));

				Driver.WriteVirtualMemory(ProcessId, GlowObjectManager + EnemyGlowIndex * 0x38 + 0x24, true, sizeof(ULONG));
				Driver.WriteVirtualMemory(ProcessId, GlowObjectManager + EnemyGlowIndex * 0x38 + 0x25, false, sizeof(ULONG));
			}
		}
		Sleep(6);
	}
	return 0;
}