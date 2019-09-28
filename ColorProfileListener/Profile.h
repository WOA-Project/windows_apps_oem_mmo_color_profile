#pragma once

#include <Windows.h>
#include <string>

struct Profile
{
	//Color values
	double UserSettingColorTargetBlueX;
	double UserSettingColorTargetBlueY;
	double UserSettingColorTargetBlueZ;
	double UserSettingColorTargetGreenX;
	double UserSettingColorTargetGreenY;
	double UserSettingColorTargetGreenZ;
	double UserSettingColorTargetRedX;
	double UserSettingColorTargetRedY;
	double UserSettingColorTargetRedZ;

	//White balance values
	double UserSettingColorTargetWhiteX;
	double UserSettingColorTargetWhiteY;
	double UserSettingColorTargetWhiteZ;

	//Currently broken
	int UserSettingColorSaturationMatrix;
	int UserSettingColorSaturationPA;

public:
	void ApplyProfile(HKEY const& key) const;
	static Profile GetNightLightProfile(double NightLightValue);
	static Profile GenerateAdvancedProfile(double Temperature, double Tint, double Saturation);
	static Profile GetVivid();
	static Profile GetCool();
	static Profile GetDefault();

	static bool SetValue(HKEY hRegistryKey, const std::wstring& valueName, const std::wstring& data);
	static bool SetValue(HKEY hRegistryKey, const std::wstring& valueName, const DWORD data);

};
