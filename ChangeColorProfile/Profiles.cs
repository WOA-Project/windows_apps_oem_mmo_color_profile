using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ColorProfileEnhancements
{

    public class Profile
    {
        Microsoft.Win32.RegistryKey LocalMachine = Microsoft.Win32.RegistryKey.OpenBaseKey(Microsoft.Win32.RegistryHive.LocalMachine, Microsoft.Win32.RegistryView.Registry64);

        // Color values
        public double UserSettingColorTargetBlueX { get; set; }
        public double UserSettingColorTargetBlueY { get; set; }
        public double UserSettingColorTargetBlueZ { get; set; }
        public double UserSettingColorTargetGreenX { get; set; }
        public double UserSettingColorTargetGreenY { get; set; }
        public double UserSettingColorTargetGreenZ { get; set; }
        public double UserSettingColorTargetRedX { get; set; }
        public double UserSettingColorTargetRedY { get; set; }
        public double UserSettingColorTargetRedZ { get; set; }

        // White balance values
        public double UserSettingColorTargetWhiteX { get; set; }
        public double UserSettingColorTargetWhiteY { get; set; }
        public double UserSettingColorTargetWhiteZ { get; set; }

        // Currently broken
        public int UserSettingColorSaturationMatrix { get; set; }
        public int UserSettingColorSaturationPA { get; set; }

        Microsoft.Win32.RegistryKey key;

        public Profile()
        {
            key = LocalMachine.OpenSubKey(@"SOFTWARE\OEM\Nokia\Display\ColorAndLight", true);
        }

        private void SetValue(string name, double value)
        {
            key.SetValue(name, Math.Round(value, 6).ToString());
        }

        private void SetValue(string name, int value)
        {
            key.SetValue(name, value);
        }

        private int GetValueInt(string name)
        {
            return (int)key.GetValue(name, 0);
        }

        public void ApplyProfile()
        {
            SetValue("UserSettingColorTargetBlueX", UserSettingColorTargetBlueX);
            SetValue("UserSettingColorTargetBlueY", UserSettingColorTargetBlueY);
            SetValue("UserSettingColorTargetBlueZ", UserSettingColorTargetBlueZ);

            SetValue("UserSettingColorTargetGreenX", UserSettingColorTargetGreenX);
            SetValue("UserSettingColorTargetGreenY", UserSettingColorTargetGreenY);
            SetValue("UserSettingColorTargetGreenZ", UserSettingColorTargetGreenZ);

            SetValue("UserSettingColorTargetRedX", UserSettingColorTargetRedX);
            SetValue("UserSettingColorTargetRedY", UserSettingColorTargetRedY);
            SetValue("UserSettingColorTargetRedZ", UserSettingColorTargetRedZ);

            SetValue("UserSettingColorTargetWhiteX", UserSettingColorTargetWhiteX);
            SetValue("UserSettingColorTargetWhiteY", UserSettingColorTargetWhiteY);
            SetValue("UserSettingColorTargetWhiteZ", UserSettingColorTargetWhiteZ);

            //SetValue("UserSettingColorSaturationMatrix", UserSettingColorSaturationMatrix);
            //SetValue("UserSettingColorSaturationPA", UserSettingColorSaturationPA);

            SetValue("UserSettingAtomicUpdate", GetValueInt("UserSettingAtomicUpdate") == 1 ? 0 : 1);
        }
    }

    public static class Profiles
    {
        public static Profile Default = new Profile()
        {
            UserSettingColorTargetBlueX = 0.1805,
            UserSettingColorTargetBlueY = 0.0722,
            UserSettingColorTargetBlueZ = 0.9505,

            UserSettingColorTargetGreenX = 0.3576,
            UserSettingColorTargetGreenY = 0.7152,
            UserSettingColorTargetGreenZ = 0.1192,

            UserSettingColorTargetRedX = 0.4123,
            UserSettingColorTargetRedY = 0.2126,
            UserSettingColorTargetRedZ = 0.0192,

            UserSettingColorTargetWhiteX = 0.95015469,
            UserSettingColorTargetWhiteY = 1.0,
            UserSettingColorTargetWhiteZ = 1.08825906,

            UserSettingColorSaturationMatrix = 0x64,
            UserSettingColorSaturationPA = 0x64
        };

        public static Profile Cool = new Profile()
        {
            UserSettingColorTargetBlueX = 0.1822,
            UserSettingColorTargetBlueY = 0.0677,
            UserSettingColorTargetBlueZ = 1.0105,

            UserSettingColorTargetGreenX = 0.3013,
            UserSettingColorTargetGreenY = 0.7206,
            UserSettingColorTargetGreenZ = 0.08235,

            UserSettingColorTargetRedX = 0.49245,
            UserSettingColorTargetRedY = 0.25015,
            UserSettingColorTargetRedZ = 0.0099,

            UserSettingColorTargetWhiteX = 0.950697,
            UserSettingColorTargetWhiteY = 1.0,
            UserSettingColorTargetWhiteZ = 1.31576,

            UserSettingColorSaturationMatrix = 0x64,
            UserSettingColorSaturationPA = 0x64
        };

        public static Profile Vivid = new Profile()
        {
            UserSettingColorTargetBlueX = 0.18305,
            UserSettingColorTargetBlueY = 0.06545,
            UserSettingColorTargetBlueZ = 1.0405,

            UserSettingColorTargetGreenX = 0.27315,
            UserSettingColorTargetGreenY = 0.7233,
            UserSettingColorTargetGreenZ = 0.063925,

            UserSettingColorTargetRedX = 0.532475,
            UserSettingColorTargetRedY = 0.268925,
            UserSettingColorTargetRedZ = 0.0052,

            UserSettingColorTargetWhiteX = 0.949097,
            UserSettingColorTargetWhiteY = 1,
            UserSettingColorTargetWhiteZ = 1.15958,

            UserSettingColorSaturationMatrix = 0x67,
            UserSettingColorSaturationPA = 0x65
        };

        public static Profile GetNightLightProfile(double NightLightValue)
        {
            Profile prof = new Profile()
            {
                UserSettingColorTargetBlueX = 0.1805,
                UserSettingColorTargetBlueY = 0.0722,
                UserSettingColorTargetBlueZ = 0.9505,

                UserSettingColorTargetGreenX = 0.3576,
                UserSettingColorTargetGreenY = 0.7152,
                UserSettingColorTargetGreenZ = 0.1192,

                UserSettingColorTargetRedX = 0.4123,
                UserSettingColorTargetRedY = 0.2126,
                UserSettingColorTargetRedZ = 0.0192,

                UserSettingColorSaturationMatrix = 0x64,
                UserSettingColorSaturationPA = 0x64
            };

            double Temperature = (NightLightValue * -212.24 + 26056) / 2;

            if (Temperature < 0 || Temperature > 100)
                Temperature = 0;

            double Tint = 50;

            prof.UserSettingColorTargetWhiteX = -0.0003 * Temperature + 0.885784 + (-0.00000068 * Temperature + 0.001968) * Tint;
            prof.UserSettingColorTargetWhiteY = 1;
            prof.UserSettingColorTargetWhiteZ = 0.008406 * Temperature + 0.542142 + (0.0000187 * Temperature + 0.001205) * Tint;

            return prof;
        }

        public static Profile GenerateAdvancedProfile(double Temperature, double Tint, double Saturation)
        {
            Profile prof = new Profile();

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

            if (Saturation < 50d)
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
                prof.UserSettingColorTargetBlueX = 0.000034 * (Saturation - 50d) + 0.1822;
                prof.UserSettingColorTargetBlueY = -0.00009 * (Saturation - 50d) + 0.0677;
                prof.UserSettingColorTargetBlueZ = 0.0012 * (Saturation - 50d) + 1.0105;

                prof.UserSettingColorTargetGreenX = -0.00113 * (Saturation - 50d) + 0.3013;
                prof.UserSettingColorTargetGreenY = 0.000108 * (Saturation - 50d) + 0.7206;
                prof.UserSettingColorTargetGreenZ = -0.00074 * (Saturation - 50d) + 0.08235;

                prof.UserSettingColorTargetRedX = 0.001601 * (Saturation - 50d) + 0.49245;
                prof.UserSettingColorTargetRedY = 0.000751 * (Saturation - 50d) + 0.25015;
                prof.UserSettingColorTargetRedZ = -0.00019 * (Saturation - 50d) + 0.0099;
            }

            return prof;
        }
    }
}
