using System;
using System.Linq;

namespace ColorProfileEnhancements
{
    class Program
    {
        static Microsoft.Win32.RegistryKey LocalMachine = Microsoft.Win32.RegistryKey.OpenBaseKey(Microsoft.Win32.RegistryHive.LocalMachine, Microsoft.Win32.RegistryView.Registry64);
        //static Microsoft.Win32.RegistryKey CurrentUser = Microsoft.Win32.RegistryKey.OpenBaseKey(Microsoft.Win32.RegistryHive.CurrentUser, Microsoft.Win32.RegistryView.Registry64);

        static void Main(string[] args)
        {
            var key = LocalMachine.OpenSubKey(@"SOFTWARE\OEM\Nokia\Display\ColorAndLight", true);

            if (args.Count() < 1)
            {
                /*var val = key.GetValue("UserSettingSelectedProfile", "Standard.icm");

                using (var ukey = CurrentUser.OpenSubKey("Software\\Microsoft\\Windows NT\\CurrentVersion\\ICM\\ProfileAssociations\\Display\\{4d36e96e-e325-11ce-bfc1-08002be10318}\\0001", true))
                {
                    string[] data = new string[0];
                    switch (val)
                    {
                        case "Standard.icm":
                            data = new string[] { "Advanced.icm", "Cool.icm", "Vivid.icm", "Standard.icm" };
                            break;
                        case "Vivid.icm":
                            data = new string[] { "Standard.icm", "Advanced.icm", "Cool.icm", "Vivid.icm" };
                            break;
                        case "Cool.icm":
                            data = new string[] { "Vivid.icm", "Standard.icm", "Advanced.icm", "Cool.icm" };
                            break;
                        case "Advanced.icm":
                            data = new string[] { "Cool.icm", "Vivid.icm", "Standard.icm", "Advanced.icm" };
                            break;
                    }
                    ukey.SetValue("ICMProfile", data);
                }*/

                return;
            }

            var lastprofile = args[0];

            if (!lastprofile.EndsWith(".icm"))
            {
                var perc = double.Parse(lastprofile);
                key.SetValue("UserSettingSelectedProfile", "Night light.icm");
                Profiles.GetNightLightProfile(perc).ApplyProfile();
                return;
            }

            key.SetValue("UserSettingSelectedProfile", lastprofile);

            switch (lastprofile)
            {
                case "Standard.icm":
                    {
                        Profiles.Default.ApplyProfile();
                        break;
                    }
                case "Vivid.icm":
                    {
                        Profiles.Vivid.ApplyProfile();
                        break;
                    }
                case "Cool.icm":
                    {
                        Profiles.Cool.ApplyProfile();
                        break;
                    }
                case "Advanced.icm":
                    {
                        int Saturation = (int)key.GetValue("UserSettingAdvancedSaturation", 25);
                        int Tint = (int)key.GetValue("UserSettingAdvancedTint", 50);
                        int Temp = (int)key.GetValue("UserSettingAdvancedTemperature", 50);

                        Profiles.GenerateAdvancedProfile(Temp, Tint, Saturation).ApplyProfile();
                        break;
                    }
            }
        }
    }
}
