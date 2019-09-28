#include "Profile.h"
#include <string>
#include <cmath>

bool Profile::SetValue(HKEY hRegistryKey, const std::wstring& valueName, const std::wstring& data)
{
	return RegSetValueEx(hRegistryKey, valueName.c_str(), NULL, REG_SZ, LPBYTE(data.c_str()), (data.size() + 1) * sizeof(wchar_t)) == ERROR_SUCCESS;
}

bool Profile::SetValue(HKEY hRegistryKey, const std::wstring& valueName, const DWORD data)
{
	return RegSetValueEx(hRegistryKey, valueName.c_str(), NULL, REG_DWORD, LPBYTE(&data), data) == ERROR_SUCCESS;
}

void Profile::ApplyProfile(HKEY const& key) const
{
	SetValue(key, L"UserSettingColorTargetBlueX", std::to_wstring(std::round(UserSettingColorTargetBlueX * 1000000) / 1000000));
	SetValue(key, L"UserSettingColorTargetBlueY", std::to_wstring(std::round(UserSettingColorTargetBlueY * 1000000) / 1000000));
	SetValue(key, L"UserSettingColorTargetBlueZ", std::to_wstring(std::round(UserSettingColorTargetBlueZ * 1000000) / 1000000));

	SetValue(key, L"UserSettingColorTargetGreenX", std::to_wstring(std::round(UserSettingColorTargetGreenX * 1000000) / 1000000));
	SetValue(key, L"UserSettingColorTargetGreenY", std::to_wstring(std::round(UserSettingColorTargetGreenY * 1000000) / 1000000));
	SetValue(key, L"UserSettingColorTargetGreenZ", std::to_wstring(std::round(UserSettingColorTargetGreenZ * 1000000) / 1000000));

	SetValue(key, L"UserSettingColorTargetRedX", std::to_wstring(std::round(UserSettingColorTargetRedX * 1000000) / 1000000));
	SetValue(key, L"UserSettingColorTargetRedY", std::to_wstring(std::round(UserSettingColorTargetRedY * 1000000) / 1000000));
	SetValue(key, L"UserSettingColorTargetRedZ", std::to_wstring(std::round(UserSettingColorTargetRedZ * 1000000) / 1000000));

	SetValue(key, L"UserSettingColorTargetWhiteX", std::to_wstring(std::round(UserSettingColorTargetWhiteX * 1000000) / 1000000));
	SetValue(key, L"UserSettingColorTargetWhiteY", std::to_wstring(std::round(UserSettingColorTargetWhiteY * 1000000) / 1000000));
	SetValue(key, L"UserSettingColorTargetWhiteZ", std::to_wstring(std::round(UserSettingColorTargetWhiteZ * 1000000) / 1000000));

	DWORD dwData = 0;
	DWORD cbData = sizeof(DWORD);

	long err = RegQueryValueEx(key, L"UserSettingAtomicUpdate", 0, 0, LPBYTE(&dwData), &cbData);

	if (err == ERROR_SUCCESS)
	{
		if (dwData == 1)
			SetValue(key, L"UserSettingAtomicUpdate", 0);
		else
			SetValue(key, L"UserSettingAtomicUpdate", 1);
	}
}

Profile Profile::GetNightLightProfile(double NightLightValue)
{
	Profile prof{
		0.1805,
		0.0722,
		0.9505,
		0.3576,
		0.7152,
		0.1192,
		0.4123,
		0.2126,
		0.0192,
		0,
		0,
		0,
		0x64,
		0x64 };

	double temperature = ((NightLightValue - 4832.0) / 212.24) - 50;

	if (temperature < -50 || temperature > 50)
		temperature = 0;
	
	const double tint = 50;

	prof.UserSettingColorTargetWhiteX = -0.0003 * temperature + 0.885784 + (-0.00000068 * temperature + 0.001968) * tint;
	prof.UserSettingColorTargetWhiteY = 1;
	prof.UserSettingColorTargetWhiteZ = 0.008406 * temperature + 0.542142 + (0.0000187 * temperature + 0.001205) * temperature;

	return prof;
}

Profile Profile::GenerateAdvancedProfile(double Temperature, double Tint, double Saturation)
{
	Profile prof{};

	prof.UserSettingColorSaturationMatrix = 0x64;
	prof.UserSettingColorSaturationPA = 0x64;

	if (Saturation > 66)
	{
		prof.UserSettingColorSaturationMatrix = 0x69;
		prof.UserSettingColorSaturationPA = 0x66;
	}

	prof.UserSettingColorTargetWhiteX = -0.0003 * Temperature + 0.885784 + (-0.00000068 * Temperature + 0.001968) * Tint;
	prof.UserSettingColorTargetWhiteY = 1;
	prof.UserSettingColorTargetWhiteZ = 0.008406 * Temperature + 0.542142 + (0.0000187 * Temperature + 0.001205) * Tint;

	if (Saturation < 50.0)
	{
		prof.UserSettingColorTargetBlueX = 0.000068 * Saturation + 0.1788;
		prof.UserSettingColorTargetBlueY = -0.00018 * Saturation + 0.0767;
		prof.UserSettingColorTargetBlueZ = 0.0024 * Saturation + 0.8905;

		prof.UserSettingColorTargetGreenX = -0.00225 * Saturation + 0.4139;
		prof.UserSettingColorTargetGreenY = 0.000216 * Saturation + 0.7098;
		prof.UserSettingColorTargetGreenZ = -0.00147 * Saturation + 0.15605;

		prof.UserSettingColorTargetRedX = 0.003202 * Saturation + 0.33235;
		prof.UserSettingColorTargetRedY = 0.001502 * Saturation + 0.17505;
		prof.UserSettingColorTargetRedZ = -0.00038 * Saturation + 0.0287;
	}
	else
	{
		prof.UserSettingColorTargetBlueX = 0.000034 * (Saturation - 50.0) + 0.1822;
		prof.UserSettingColorTargetBlueY = -0.00009 * (Saturation - 50.0) + 0.0677;
		prof.UserSettingColorTargetBlueZ = 0.0012 * (Saturation - 50.0) + 1.0105;

		prof.UserSettingColorTargetGreenX = -0.00113 * (Saturation - 50.0) + 0.3013;
		prof.UserSettingColorTargetGreenY = 0.000108 * (Saturation - 50.0) + 0.7206;
		prof.UserSettingColorTargetGreenZ = -0.00074 * (Saturation - 50.0) + 0.08235;

		prof.UserSettingColorTargetRedX = 0.001601 * (Saturation - 50.0) + 0.49245;
		prof.UserSettingColorTargetRedY = 0.000751 * (Saturation - 50.0) + 0.25015;
		prof.UserSettingColorTargetRedZ = -0.00019 * (Saturation - 50.0) + 0.0099;
	}

	return prof;
}


Profile Profile::GetVivid()
{
	return Profile{
		0.18305,
		0.06545,
		1.0405,
		0.27315,
		0.7233,
		0.063925,
		0.532475,
		0.268925,
		0.0052,
		0.949097,
		1,
		1.15958,
		0x67,
		0x65
	};
}

Profile Profile::GetCool()
{
	return Profile{
		0.1822,
		0.0677,
		1.0105,
		0.3013,
		0.7206,
		0.08235,
		0.49245,
		0.25015,
		0.0099,
		0.950697,
		1.0,
		1.31576,
		0x64,
		0x64
	};
}

Profile Profile::GetDefault()
{
	return Profile{
		0.1805,
		0.0722,
		0.9505,
		0.3576,
		0.7152,
		0.1192,
		0.4123,
		0.2126,
		0.0192,
		0.95015469,
		1.0,
		1.08825906,
		0x64,
		0x64
	};
}
