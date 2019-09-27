// ColorProfileListener.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <winreg.h>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>

#define PROFILE_KEY_PATH L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ICM\\ProfileAssociations\\Display\\{4d36e96e-e325-11ce-bfc1-08002be10318}\\0001"
#define COLOR_AND_LIGHT_KEY_PATH L"SOFTWARE\\OEM\\Nokia\\Display\\ColorAndLight"

#define BLUELIGHT_REDUCTION_KEY_PATH L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CloudStore\\Store\\DefaultAccount\\Current\\default$windows.data.bluelightreduction.settings\\windows.data.bluelightreduction.settings"
#define BLUELIGHT_REDUCTION_STATE_KEY_PATH L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CloudStore\\Store\\DefaultAccount\\Current\\default$windows.data.bluelightreduction.bluelightreductionstate\\windows.data.bluelightreduction.bluelightreductionstate"

#define CHANGE_COLOR_PROFILE_PATH L"\\Windows\\OEM\\ChangeColorProfile.exe"

void CheckForBluelightReductionState();
void CheckForSettings();
void ChangedEventSignal();
void NewStatus(bool, bool, int);

void NightModePreviewModeExited();
void NightModePreviewModeEntered(int);
void NightModePreviewSliderUpdated(int);
void NightModeSliderUpdated(int);
void NightModeEnabled(int);
void NightModeDisabled();

void CheckForProfileChangeFromExternal();
void CheckForProfileChangeFromInternal();

unsigned long long ExecuteProcess(std::wstring FullPathToExe, std::wstring Parameters, unsigned long SecondsToWait);

std::wstring ReadSelectedProfile();

void ProvisionDefaultProfileData();
void ProvisionProfileListData();

void EnableNightLight(unsigned long value);
void DisableNightLight();

int main();

std::wstring _lastfoundprofile = std::wstring(L"");
bool _nightlight = false;

// Back store to prevent multiple signaling.
bool _isEnabled = false; //default value.
bool _isPreviewing = false; //default value.
bool _loaded = false;
int _valueSlider;

HKEY bluelightreductionStateKey = nullptr;
HKEY settingsKey = nullptr;

HKEY profileKey = nullptr;
HKEY colorAndLightKey = nullptr;

// From http://goffconcepts.com/techarticles/createprocess.html

unsigned long long ExecuteProcess(std::wstring FullPathToExe, std::wstring Parameters, unsigned long SecondsToWait)
{
	unsigned long long iMyCounter = 0, iReturnVal = 0, iPos = 0;
	unsigned long long dwExitCode = 0;
	std::wstring sTempStr = L"";

	// Add a space to the beginning of the Parameters
	if (Parameters.size() != 0)
	{
		if (Parameters[0] != L' ')
		{
			Parameters.insert(0, L" ");
		}
	}

	// The first parameter needs to be the exe itself
	sTempStr = FullPathToExe;
	iPos = sTempStr.find_last_of(L"\\");
	sTempStr.erase(0, iPos + 1);
	Parameters = sTempStr.append(Parameters);

	// CreateProcessW can modify Parameters thus we allocate needed memory
	wchar_t* pwszParam = new wchar_t[Parameters.size() + 1];
	if (pwszParam == 0)
	{
		return 1;
	}
	const wchar_t* pchrTemp = Parameters.c_str();
	wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp);

	/* CreateProcess API initialization */
	STARTUPINFOW siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	if (CreateProcessW(const_cast<LPCWSTR>(FullPathToExe.c_str()), pwszParam, 0, 0, false, CREATE_DEFAULT_ERROR_MODE, 0, 0, &siStartupInfo, &piProcessInfo) != false)
	{
		// Watch the process.
		dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, SecondsToWait * 1000lu);
	}
	else
	{
		// CreateProcess failed
		iReturnVal = GetLastError();
	}

	// Free memory
	delete[]pwszParam;
	pwszParam = 0;

	// Release handles
	CloseHandle(piProcessInfo.hProcess);
	CloseHandle(piProcessInfo.hThread);

	return iReturnVal;
}

std::wstring ReadSelectedProfile()
{
	unsigned long type, size;
	std::vector<std::wstring> target;
	int count = -1;

	long nError = RegQueryValueExW(profileKey, L"ICMProfile", NULL, &type, NULL, &size);
	if (nError != ERROR_SUCCESS)
	{
		return L"";
	}

	if (type == REG_MULTI_SZ)
	{
		std::vector<wchar_t> temp(size / sizeof(wchar_t));

		nError = RegQueryValueExW(profileKey, L"ICMProfile", NULL, NULL, reinterpret_cast<LPBYTE>(&temp[0]), &size);
		if (nError != ERROR_SUCCESS)
		{
			return L"";
		}

		unsigned long long index = 0;
		unsigned long long len = wcslen(&temp[0]);

		while (len > 0)
		{
			target.push_back(&temp[index]);
			index += len + 1;
			len = wcslen(&temp[index]);
			count++;
		}
	}

	if (count != -1)
	{
		return target[count];
	}

	return L"";
}

void ProvisionDefaultProfileData()
{
	wchar_t buf[255] = { 0 };
	unsigned long dwType = 0;
	unsigned long dwBufSize = sizeof(buf);

	HKEY hProfileKey;
	long nError = RegOpenKeyEx(HKEY_CURRENT_USER, PROFILE_KEY_PATH, NULL, KEY_ALL_ACCESS, &hProfileKey);

	if (nError == ERROR_FILE_NOT_FOUND)
	{
		RegCreateKeyEx(HKEY_CURRENT_USER, PROFILE_KEY_PATH, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hProfileKey, NULL);

		const wchar_t sz[] = L"Advanced.icm\0Cool.icm\0Vivid.icm\0Standard.icm\0";

		RegSetValueEx(hProfileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
		RegSetValueEx(hProfileKey, L"ICMProfileBackup", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));

		RegSetValueEx(hProfileKey, L"ICMProfileAC", NULL, REG_MULTI_SZ, NULL, 0);
		RegSetValueEx(hProfileKey, L"ICMProfileSnapshot", NULL, REG_MULTI_SZ, NULL, 0);
		RegSetValueEx(hProfileKey, L"ICMProfileSnapshotAC", NULL, REG_MULTI_SZ, NULL, 0);

		unsigned long data = 1;
		RegSetValueEx(hProfileKey, L"UsePerUserProfiles", NULL, REG_DWORD, (LPBYTE)&data, sizeof(unsigned long));
	}

	dwBufSize = 0;
	RegQueryValueEx(hProfileKey, L"ICMProfile", NULL, &dwType, (BYTE*)buf, &dwBufSize);
	if (dwBufSize == 0)
	{
		const wchar_t sz[] = L"Advanced.icm\0Cool.icm\0Vivid.icm\0Standard.icm\0";

		RegSetValueEx(hProfileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
		RegSetValueEx(hProfileKey, L"ICMProfileBackup", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));

		RegSetValueEx(hProfileKey, L"ICMProfileAC", NULL, REG_MULTI_SZ, NULL, 0);
		RegSetValueEx(hProfileKey, L"ICMProfileSnapshot", NULL, REG_MULTI_SZ, NULL, 0);
		RegSetValueEx(hProfileKey, L"ICMProfileSnapshotAC", NULL, REG_MULTI_SZ, NULL, 0);

		unsigned long data = 1;
		RegSetValueEx(hProfileKey, L"UsePerUserProfiles", NULL, REG_DWORD, (LPBYTE)&data, sizeof(unsigned long));
	}

	RegCloseKey(hProfileKey);
}

void ProvisionProfileListData()
{
	wchar_t buf[255] = { 0 };
	unsigned long dwType = 0;
	unsigned long dwBufSize = sizeof(buf);

	RegQueryValueEx(colorAndLightKey, L"UserSettingSelectedProfile", NULL, &dwType, (BYTE*)buf, &dwBufSize);

	std::wstring profile = std::wstring(buf);

	if (profile == L"Night light.icm")
	{
		_nightlight = true;

		const wchar_t sz[] = L"Night light.icm\0";
		RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));

		return;
	}
	else
	{
		_nightlight = false;
	}

	if (profile != L"")
	{
		if (profile == _lastfoundprofile)
		{
			return;
		}

		_lastfoundprofile = profile;

		if (profile == L"Standard.icm")
		{
			const wchar_t sz[] = L"Advanced.icm\0Cool.icm\0Vivid.icm\0Standard.icm\0";

			RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
			RegSetValueEx(profileKey, L"ICMProfileBackup", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
		}
		if (profile == L"Vivid.icm")
		{
			const wchar_t sz[] = L"Standard.icm\0Advanced.icm\0Cool.icm\0Vivid.icm\0";

			RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
			RegSetValueEx(profileKey, L"ICMProfileBackup", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
		}
		if (profile == L"Cool.icm")
		{
			const wchar_t sz[] = L"Vivid.icm\0Standard.icm\0Advanced.icm\0Cool.icm\0";

			RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
			RegSetValueEx(profileKey, L"ICMProfileBackup", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
		}
		if (profile == L"Advanced.icm")
		{
			const wchar_t sz[] = L"Cool.icm\0Vivid.icm\0Standard.icm\0Advanced.icm\0";

			RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
			RegSetValueEx(profileKey, L"ICMProfileBackup", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));
		}
	}
}

void EnableNightLight(unsigned long value)
{
	_nightlight = true;

	const wchar_t sz[] = L"Night light.icm\0";
	RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));

	ExecuteProcess(CHANGE_COLOR_PROFILE_PATH, std::to_wstring(value), 2);
}

void DisableNightLight()
{
	wchar_t buf[255] = { 0 };
	unsigned long dwType = 0;
	unsigned long dwBufSize = sizeof(buf);

	RegQueryValueEx(profileKey, L"ICMProfileBackup", NULL, &dwType, (BYTE*)buf, &dwBufSize);
	RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)buf, dwBufSize);

	std::wstring tmpstring = ReadSelectedProfile();

	_lastfoundprofile = tmpstring;
	ExecuteProcess(CHANGE_COLOR_PROFILE_PATH, _lastfoundprofile, 2);

	_nightlight = false;
}

int main()
{
	ProvisionDefaultProfileData();

	RegOpenKeyEx(HKEY_CURRENT_USER, BLUELIGHT_REDUCTION_KEY_PATH, 0, KEY_NOTIFY | KEY_READ, &settingsKey);
	RegOpenKeyEx(HKEY_CURRENT_USER, BLUELIGHT_REDUCTION_STATE_KEY_PATH, 0, KEY_NOTIFY | KEY_READ, &bluelightreductionStateKey);

	RegOpenKeyEx(HKEY_CURRENT_USER, PROFILE_KEY_PATH, 0, KEY_ALL_ACCESS, &profileKey);
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, COLOR_AND_LIGHT_KEY_PATH, 0, KEY_ALL_ACCESS, &colorAndLightKey);

	ChangedEventSignal();

	std::thread t1(CheckForBluelightReductionState);
	std::thread t2(CheckForSettings);

	std::thread t3(CheckForProfileChangeFromExternal);
	std::thread t4(CheckForProfileChangeFromInternal);

	t1.join();
	t2.join();

	t3.join();
	t4.join();

	/*while (true)
	{
		if (!_nightlight)
		{
			std::wstring tmpstring = ReadSelectedProfile();

			if (tmpstring == L"Night light.icm")
			{
				_nightlight = true;

				const wchar_t sz[] = L"Night light.icm\0";
				RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));

				continue;
			}

			if (_wcsicmp(_lastfoundprofile.c_str(), tmpstring.c_str()))
			{
				_lastfoundprofile = tmpstring;
				ExecuteProcess(CHANGE_COLOR_PROFILE_PATH, _lastfoundprofile, 2);
			}
		}

		ProvisionProfileListData();

		Sleep(1000);
	}*/
}

void CheckForProfileChangeFromExternal()
{
	HANDLE hEvent = CreateEvent(NULL, true, false, NULL);

	RegNotifyChangeKeyValue(colorAndLightKey, true, REG_NOTIFY_CHANGE_LAST_SET, hEvent, true);

	while (true)
	{
		if (WaitForSingleObject(hEvent, INFINITE) == WAIT_FAILED)
		{
			exit(0);
		}

		ProvisionProfileListData();

		RegNotifyChangeKeyValue(colorAndLightKey, false, REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_SECURITY, hEvent, true);
	}
}

void CheckForProfileChangeFromInternal()
{
	HANDLE hEvent = CreateEvent(NULL, true, false, NULL);

	RegNotifyChangeKeyValue(profileKey, true, REG_NOTIFY_CHANGE_LAST_SET, hEvent, true);

	while (true)
	{
		if (WaitForSingleObject(hEvent, INFINITE) == WAIT_FAILED)
		{
			exit(0);
		}

		if (!_nightlight)
		{
			std::wstring tmpstring = ReadSelectedProfile();

			if (tmpstring == L"Night light.icm")
			{
				_nightlight = true;

				const wchar_t sz[] = L"Night light.icm\0";
				RegSetValueEx(profileKey, L"ICMProfile", NULL, REG_MULTI_SZ, (LPBYTE)sz, sizeof(sz));

				continue;
			}

			if (_wcsicmp(_lastfoundprofile.c_str(), tmpstring.c_str()))
			{
				_lastfoundprofile = tmpstring;
				ExecuteProcess(CHANGE_COLOR_PROFILE_PATH, _lastfoundprofile, 2);
			}
		}

		RegNotifyChangeKeyValue(profileKey, false, REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_SECURITY, hEvent, true);
	}
}

void CheckForSettings()
{
	HANDLE hEvent = CreateEvent(NULL, true, false, NULL);

	RegNotifyChangeKeyValue(settingsKey, true, REG_NOTIFY_CHANGE_LAST_SET, hEvent, true);

	while (true)
	{
		if (WaitForSingleObject(hEvent, INFINITE) == WAIT_FAILED)
		{
			exit(0);
		}

		ChangedEventSignal();
		RegNotifyChangeKeyValue(settingsKey, false, REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_SECURITY, hEvent, true);
	}
}

void CheckForBluelightReductionState()
{
	HANDLE hEvent = CreateEvent(NULL, true, false, NULL);

	RegNotifyChangeKeyValue(bluelightreductionStateKey, false, REG_NOTIFY_CHANGE_LAST_SET, hEvent, true);

	while (true)
	{
		if (WaitForSingleObject(hEvent, INFINITE) == WAIT_FAILED)
		{
			exit(0);
		}

		ChangedEventSignal();
		RegNotifyChangeKeyValue(bluelightreductionStateKey, false, REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_SECURITY, hEvent, true);
	}
}

void ChangedEventSignal()
{
	unsigned char dwSettingsArray[100];  //dwReturn[1]
	unsigned long dwSettingsBufSize = sizeof(dwSettingsArray);

	unsigned char dwStateArray[100];  //dwReturn[1]
	unsigned long dwStateBufSize = sizeof(dwStateArray);

	long err = RegQueryValueEx(settingsKey, L"Data", 0, 0, LPBYTE(dwSettingsArray), &dwSettingsBufSize);
	long err2 = RegQueryValueEx(bluelightreductionStateKey, L"Data", 0, 0, LPBYTE(dwStateArray), &dwStateBufSize);

	if (err + err2 == ERROR_SUCCESS && dwSettingsBufSize > 15 && dwStateBufSize > 10)
	{
		unsigned char signaturePreviewSlider[] = { 0xC2, 0x46, 0x01 };

		unsigned char offsetVal = dwSettingsArray[18];
		unsigned char val[] { dwSettingsArray[dwSettingsBufSize - 7], dwSettingsArray[dwSettingsBufSize - 6], dwSettingsArray[dwSettingsBufSize - 5] };

		bool isPreviewing = memcmp(&val, &signaturePreviewSlider, sizeof(signaturePreviewSlider)) == 0;

		int value = 0;

		//if (isPreviewing)
		//{
		//	value = (dwSettingsArray[offsetVal] << 8) | dwSettingsArray[offsetVal - 1];
		//}
		//else
		//{
			value = (dwSettingsArray[offsetVal + 3] << 8) | dwSettingsArray[offsetVal + 2];
		//}

		bool isEnabled = dwStateArray[18] == 21;

		NewStatus(isEnabled, isPreviewing, value);
	}
}

void NewStatus(bool isEnabled, bool isPreviewing, int valueSlider)
{
	bool wasnotloaded = !_loaded;
	_loaded = true;
	
	if (wasnotloaded || _isEnabled != isEnabled)
	{
		_isEnabled = isEnabled;
		_valueSlider = valueSlider;

		if (isEnabled)
		{
			NightModeEnabled(valueSlider);
		}
		else
		{
			NightModeDisabled();
			return; //No need to continue with the other code.
		}
	}

	if (wasnotloaded || _valueSlider != valueSlider)
	{
		_valueSlider = valueSlider;
		if (isPreviewing && !isEnabled)
		{
			NightModePreviewSliderUpdated(valueSlider);
		}
		else if (isEnabled)
		{
			NightModeSliderUpdated(valueSlider);
		}
	}
	else
	{
		if (isPreviewing && !isEnabled)
		{
			NightModePreviewModeEntered(valueSlider);
		}
		else if (!isPreviewing && !isEnabled)
		{
			NightModePreviewModeExited();
		}
	}

}

//WITH NIGHT MODE ENABLED

void NightModeSliderUpdated(int valueSlider)
{
	//This is when you move the slider (when night mode is enabled)
	//printf("Normal slider updated\n\n");

	EnableNightLight(valueSlider);
}

void NightModeEnabled(int valueSlider)
{
	//This is when you enabled night mode
	//printf("Enabled\n\n");

	EnableNightLight(valueSlider);
}

void NightModeDisabled()
{
	//This is when you disable night mode

	DisableNightLight();
}



//WITH NIGHT MODE DISABLED

void NightModePreviewSliderUpdated(int valueSlider)
{
	//This is when you move the slider (while in preview mode)
	//printf("Preview slider updated\n\n");

	EnableNightLight(valueSlider);
}

void NightModePreviewModeEntered(int valueSlider)
{
	//This is when you press the slider (entering preview mode)
	//printf("Preview mode entered\n\n");

	EnableNightLight(valueSlider);
}

void NightModePreviewModeExited()
{
	//This is when you release the slider (leaving preview mode)
	//printf("Preview mode exited\n\n");

	DisableNightLight();
}