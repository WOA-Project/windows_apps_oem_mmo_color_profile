// ColorProfileListener.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <winreg.h>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include "Profile.h"

#define PROFILE_KEY_PATH L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ICM\\ProfileAssociations\\Display\\{4d36e96e-e325-11ce-bfc1-08002be10318}\\0001"
#define COLOR_AND_LIGHT_KEY_PATH L"SOFTWARE\\OEM\\Nokia\\Display\\ColorAndLight"

#define BLUELIGHT_REDUCTION_KEY_PATH L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CloudStore\\Store\\DefaultAccount\\Current\\default$windows.data.bluelightreduction.settings\\windows.data.bluelightreduction.settings"
#define BLUELIGHT_REDUCTION_STATE_KEY_PATH L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CloudStore\\Store\\DefaultAccount\\Current\\default$windows.data.bluelightreduction.bluelightreductionstate\\windows.data.bluelightreduction.bluelightreductionstate"

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

void ChangeColorProfile(std::wstring);
void ChangeColorProfileNightLight(double);
bool ends_with(std::wstring const&, std::wstring const&);

void CheckForProfileChangeFromExternal();
void CheckForProfileChangeFromInternal();

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

	if (!profile.empty())
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

	ChangeColorProfileNightLight(value);
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
	ChangeColorProfile(_lastfoundprofile);

	_nightlight = false;
}

int main()
{
	ProvisionDefaultProfileData();

	RegOpenKeyEx(HKEY_CURRENT_USER, BLUELIGHT_REDUCTION_KEY_PATH, 0, KEY_NOTIFY | KEY_READ, &settingsKey);
	RegOpenKeyEx(HKEY_CURRENT_USER, BLUELIGHT_REDUCTION_STATE_KEY_PATH, 0, KEY_NOTIFY | KEY_READ, &bluelightreductionStateKey);

	RegOpenKeyEx(HKEY_CURRENT_USER, PROFILE_KEY_PATH, 0, KEY_ALL_ACCESS, &profileKey);
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, COLOR_AND_LIGHT_KEY_PATH, 0, KEY_ALL_ACCESS | KEY_WRITE, &colorAndLightKey);

	ChangedEventSignal();

	std::thread t1(CheckForBluelightReductionState);
	std::thread t2(CheckForSettings);

	std::thread t3(CheckForProfileChangeFromExternal);
	std::thread t4(CheckForProfileChangeFromInternal);

	t1.join();
	t2.join();

	t3.join();
	t4.join();
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
				ChangeColorProfile(_lastfoundprofile);
			}
		}

		RegNotifyChangeKeyValue(profileKey, false, REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_SECURITY, hEvent, true);
	}
}

void ChangeColorProfileNightLight(double value)
{
	const std::wstring profileName = L"Night light.icm";
	RegSetValueEx(colorAndLightKey, L"UserSettingSelectedProfile", NULL, REG_SZ, LPBYTE(profileName.c_str()), (profileName.size() + 1) * sizeof(wchar_t));

	std::wstring valstr = std::to_wstring(value);
	RegSetValueEx(colorAndLightKey, L"UserSettingNightLightWat", NULL, REG_SZ, LPBYTE(valstr.c_str()), (valstr.size() + 1) * sizeof(wchar_t));

	const Profile prof = Profile::GetNightLightProfile(value);
	prof.ApplyProfile(colorAndLightKey);
}

void ChangeColorProfile(std::wstring lastprofile)
{
	Profile::SetValue(colorAndLightKey, L"UserSettingSelectedProfile", lastprofile);
	ProvisionProfileListData();

	if (lastprofile == L"Standard.icm")
	{
		const Profile prof = Profile::GetDefault();
		prof.ApplyProfile(colorAndLightKey);
	}
	else if (lastprofile == L"Vivid.icm")
	{
		const Profile prof = Profile::GetVivid();
		prof.ApplyProfile(colorAndLightKey);
	}
	else if (lastprofile == L"Cool.icm")
	{
		const Profile prof = Profile::GetCool();
		prof.ApplyProfile(colorAndLightKey);
	}
	else if (lastprofile == L"Advanced.icm")
	{
		int saturation = 0;
		int tint = 0;
		int temp = 0;

		unsigned long dwData = 0;
		unsigned long cbData = sizeof(unsigned long);

		long err = RegQueryValueEx(colorAndLightKey, L"UserSettingAdvancedSaturation", 0, 0, LPBYTE(&dwData), &cbData);

		if (err == ERROR_SUCCESS)
			saturation = dwData;
		else
			saturation = 25;

		err = RegQueryValueEx(colorAndLightKey, L"UserSettingAdvancedTint", 0, 0, LPBYTE(&dwData), &cbData);

		if (err == ERROR_SUCCESS)
			tint = dwData;
		else
			tint = 50;

		err = RegQueryValueEx(colorAndLightKey, L"UserSettingAdvancedTemperature", 0, 0, LPBYTE(&dwData), &cbData);

		if (err == ERROR_SUCCESS)
			temp = dwData;
		else
			temp = 50;

		
		const auto prof = Profile::GenerateAdvancedProfile(temp, tint, saturation);
		prof.ApplyProfile(colorAndLightKey);
	}
}

//NIGHT MODE RELATED SETTINGS -- TODO --

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

		/*
		    WARNING: For whom it may concern, this is currently unexplainable.

			For some extraordinary reason, on some devices the slider value is
			storred using an offset of 0 bytes in the data buffer.
			On all of my devices this offset is set to 2, for Abdel (@ADeltaX)
			it's set to 0. I think given the structure of the buffer having an
			offset of 2 is actually expected and normal behavior, and not
			having one is an oddity no one can explain.

			If you're having issues with the value being stuck, you now know
			what to do.
		*/

		int offset = 2;

		if (isPreviewing)
		{
			value = int((unsigned char)(0) << 24 | (unsigned char)(0) << 16 | dwSettingsArray[offsetVal + offset] << 8 | dwSettingsArray[offsetVal + offset - 1]);
		}
		else
		{
			value = int((unsigned char)(0) << 24 | (unsigned char)(0) << 16 | dwSettingsArray[offsetVal + offset + 3] << 8 | dwSettingsArray[offsetVal + offset + 2]);
		}

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