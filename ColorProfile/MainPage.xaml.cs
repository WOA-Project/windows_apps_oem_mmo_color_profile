using System;
using Windows.ApplicationModel.Core;
using Windows.UI;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.Storage;
using RegistryRT;

namespace ColorProfile
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private ApplicationDataContainer localSettings = ApplicationData.Current.LocalSettings;

        private Registry regrt = new Registry();

        private string key = @"SOFTWARE\OEM\Nokia\Display\ColorAndLight";

        private bool initialized = false;

        public MainPage()
        {
            regrt.InitNTDLLEntryPoints();
            this.InitializeComponent();
            ImageViewScrollViewer.ChangeView(null, ImageView.Width / 2, null);
            RetrieveSettings();

            Window.Current.SizeChanged += Current_SizeChanged;
            Loaded += MainPage_Loaded;
            CoreApplication.GetCurrentView().TitleBar.ExtendViewIntoTitleBar = true;
            ApplicationView.GetForCurrentView().TitleBar.ButtonBackgroundColor = Colors.Transparent;
            ApplicationView.GetForCurrentView().TitleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
        }

        private void SetValue(string name, int value)
        {
            regrt.WriteValue(RegistryHive.HKEY_LOCAL_MACHINE, key, name, BitConverter.GetBytes(value), RegistryType.Integer);
        }

        private void SetValue(string name, string value)
        {
            regrt.WriteValue(RegistryHive.HKEY_LOCAL_MACHINE, key, name, System.Text.Encoding.Unicode.GetBytes(value + "\0"), RegistryType.String);
        }

        private string GetValueStr(string name)
        {
            RegistryType type;
            byte[] buffer;

            regrt.QueryValue(RegistryHive.HKEY_LOCAL_MACHINE, key, name, out type, out buffer);
            if (buffer == null)
                return "";

            return System.Text.Encoding.Unicode.GetString(buffer, 0, buffer.Length - 2);
        }

        private int GetValueInt(string name)
        {
            RegistryType type;
            byte[] buffer;

            regrt.QueryValue(RegistryHive.HKEY_LOCAL_MACHINE, key, name, out type, out buffer);
            if (buffer == null)
                return 0;
            return BitConverter.ToInt32(buffer, 0);
        }

        private void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            Picture1.Width = ActualWidth;
            Picture2.Width = ActualWidth;
            Picture3.Width = ActualWidth;
            Picture4.Width = ActualWidth;
        }

        private void Current_SizeChanged(object sender, Windows.UI.Core.WindowSizeChangedEventArgs e)
        {
            Picture1.Width = e.Size.Width;
            Picture2.Width = e.Size.Width;
            Picture3.Width = e.Size.Width;
            Picture4.Width = e.Size.Width;
        }

        private void RetrieveSettings()
        {
            bool blockUI = false;

            SunlightReadabilityToggle.IsOn = GetValueInt("UserSettingSreEnabled") == 1;
            BatterySaverBrightnessToggle.IsOn = GetValueInt("UserSettingBsmDimmingEnabled") == 1;

            var prof = GetValueStr("UserSettingSelectedProfile");
            if (!string.IsNullOrEmpty(prof))
            {
                switch (prof)
                {
                    case "Standard.icm":
                        {
                            localSettings.Values["SelectedProfileIndex"] = 0;
                            ProfileSelector.SelectedIndex = 0;
                            AdvancedSliderPanel.Visibility = Visibility.Collapsed;
                            break;
                        }
                    case "Vivid.icm":
                        {
                            localSettings.Values["SelectedProfileIndex"] = 1;
                            ProfileSelector.SelectedIndex = 1;
                            AdvancedSliderPanel.Visibility = Visibility.Collapsed;
                            break;
                        }
                    case "Cool.icm":
                        {
                            localSettings.Values["SelectedProfileIndex"] = 2;
                            ProfileSelector.SelectedIndex = 2;
                            AdvancedSliderPanel.Visibility = Visibility.Collapsed;
                            break;
                        }
                    case "Advanced.icm":
                        {
                            localSettings.Values["SelectedProfileIndex"] = 3;
                            ProfileSelector.SelectedIndex = 3;
                            AdvancedSliderPanel.Visibility = Visibility.Visible;
                            break;
                        }
                    case "Night light.icm":
                        {
                            blockUI = true;
                            ProfileSelector.IsEnabled = false;
                            ProfileSelector.Items.Add(new ComboBoxItem() { Content = "Night light" });
                            ProfileSelector.SelectedIndex = 4;
                            AdvancedSliderPanel.Visibility = Visibility.Collapsed;
                            break;
                        }
                }
            }
            else
            {
                int? SelectedProfileIndex = (int?)localSettings.Values["SelectedProfileIndex"];

                if (SelectedProfileIndex.HasValue)
                {
                    ProfileSelector.SelectedIndex = SelectedProfileIndex.Value;
                    if (ProfileSelector.SelectedIndex == 3)
                    {
                        AdvancedSliderPanel.Visibility = Visibility.Visible;
                    }
                }
                else
                {
                    localSettings.Values["SelectedProfileIndex"] = 0;
                }
            }

            if (blockUI)
            {
                initialized = true;
                return;
            }

            double? TemperaturePercentage = (double?)localSettings.Values["TemperaturePercentage"];

            if (TemperaturePercentage.HasValue)
            {
                TemperatureSlider.Value = TemperaturePercentage.Value;
            }
            else
            {
                localSettings.Values["TemperaturePercentage"] = 50d;
            }

            double? TintPercentage = (double?)localSettings.Values["TintPercentage"];

            if (TintPercentage.HasValue)
            {
                TintSlider.Value = TintPercentage.Value;
            }
            else
            {
                localSettings.Values["TintPercentage"] = 50d;
            }

            double? SaturationPercentage = (double?)localSettings.Values["SaturationPercentage"];

            if (SaturationPercentage.HasValue)
            {
                SaturationSlider.Value = SaturationPercentage.Value;
            }
            else
            {
                localSettings.Values["SaturationPercentage"] = 25d;
            }

            initialized = true;
        }

        private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!initialized)
                return;

            try
            {
                localSettings.Values["SelectedProfileIndex"] = (sender as ComboBox).SelectedIndex;

                switch ((sender as ComboBox).SelectedIndex)
                {
                    case 0:
                        SetValue("UserSettingSelectedProfile", "Standard.icm");
                        AdvancedSliderPanel.Visibility = Visibility.Collapsed;
                        Profiles.Default.ApplyProfile();
                        break;
                    case 1:
                        SetValue("UserSettingSelectedProfile", "Vivid.icm");
                        AdvancedSliderPanel.Visibility = Visibility.Collapsed;
                        Profiles.Vivid.ApplyProfile();
                        break;
                    case 2:
                        SetValue("UserSettingSelectedProfile", "Cool.icm");
                        AdvancedSliderPanel.Visibility = Visibility.Collapsed;
                        Profiles.Cool.ApplyProfile();
                        break;
                    case 3:
                        SetValue("UserSettingSelectedProfile", "Advanced.icm");

                        SetValue("UserSettingAdvancedSaturation", Convert.ToInt32(SaturationSlider.Value));
                        SetValue("UserSettingAdvancedTint", Convert.ToInt32(TintSlider.Value));
                        SetValue("UserSettingAdvancedTemperature", Convert.ToInt32(TemperatureSlider.Value));

                        AdvancedSliderPanel.Visibility = Visibility.Visible;
                        Profiles.GenerateAdvancedProfile(TemperatureSlider.Value, TintSlider.Value, SaturationSlider.Value).ApplyProfile();
                        break;
                }

                if ((sender as ComboBox).SelectedIndex == 3)
                    AdvancedSliderPanel.Visibility = Visibility.Visible;
                else
                    AdvancedSliderPanel.Visibility = Visibility.Collapsed;
            }
            catch
            {

            }
        }

        private void TemperatureSlider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            if (!initialized)
                return;

            localSettings.Values["TemperaturePercentage"] = TemperatureSlider.Value;

            Profiles.GenerateAdvancedProfile(Convert.ToInt32(TemperatureSlider.Value), Convert.ToInt32(TintSlider.Value), Convert.ToInt32(SaturationSlider.Value)).ApplyProfile();
        }

        private void TintSlider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            if (!initialized)
                return;

            localSettings.Values["TintPercentage"] = TemperatureSlider.Value;

            Profiles.GenerateAdvancedProfile(Convert.ToInt32(TemperatureSlider.Value), Convert.ToInt32(TintSlider.Value), Convert.ToInt32(SaturationSlider.Value)).ApplyProfile();
        }

        private void SaturationSlider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            if (!initialized)
                return;

            localSettings.Values["SaturationPercentage"] = TemperatureSlider.Value;

            Profiles.GenerateAdvancedProfile(Convert.ToInt32(TemperatureSlider.Value), Convert.ToInt32(TintSlider.Value), Convert.ToInt32(SaturationSlider.Value)).ApplyProfile();
        }

        private void BatterySaverBrightnessToggle_Toggled(object sender, RoutedEventArgs e)
        {
            if (!initialized)
                return;

            SetValue("UserSettingBsmDimmingEnabled", BatterySaverBrightnessToggle.IsOn ? 1 : 0);
        }

        private void SunlightReadabilityToggle_Toggled(object sender, RoutedEventArgs e)
        {
            if (!initialized)
                return;

            SetValue("UserSettingSreEnabled", SunlightReadabilityToggle.IsOn ? 1 : 0);
        }
    }
}
