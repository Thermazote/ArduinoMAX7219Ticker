using Plugin.BluetoothClassic.Abstractions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace MAX72xx_Management
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class MainContent : ContentPage
    {
        private Color[,,] btnColors;
        const int MATRIX_COUNT = 4;
        const int MATRIX_HEIGHT = 8;
        const int MATRIX_WIDTH = 8;
        const int OFFSET_DEFAULT = 0;
        const byte OFF = 0;
        const byte ON = 1;
        private int Brightness { get; set; } = 7;
        private int Speed { get; set; } = 7;

        public MainContent()
        {
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);

            InitializeComponent();

            btnColors = new Color[MATRIX_COUNT, MATRIX_HEIGHT, MATRIX_WIDTH];
            for (int i = 0; i < MATRIX_COUNT; i++)
                for (int j = 0; j < MATRIX_HEIGHT; j++)
                    for (int k = 0; k < MATRIX_WIDTH; k++)
                        btnColors[i, j, k] = Color.FromHex("#203126");
        }

        private void OnMatrixToggled(object sender, ToggledEventArgs args)
        {
            TransmitDataToBluetoothDevice();
        }

        private void OnModeChanged(object sender, CheckedChangedEventArgs args)
        {
            EntryText.IsEnabled = args.Value;
            if (args.Value == false)
                EntryText.Text = string.Empty;

            TransmitDataToBluetoothDevice(TextOrMatrixChanged: true);
        }

        private void EntryText_Unfocused(object sender, FocusEventArgs e)
        {
            TransmitDataToBluetoothDevice(TextOrMatrixChanged: true);
        }

        private void OnStaticModeToggled(object sender, ToggledEventArgs args)
        {
            TransmitDataToBluetoothDevice();
        }

        private void OnBrightnessChanged(object sender, ValueChangedEventArgs args)
        {
            BrightnessSlider.Value = Math.Round(args.NewValue);
            
            if (Math.Abs(BrightnessSlider.Value - Brightness) >= 1)
            {
                Brightness = (int)BrightnessSlider.Value;
                BrightnessLabel.Text = $"Brightness: {Brightness}";

                TransmitDataToBluetoothDevice();
            }
        }

        private void OnSpeedChanged(object sender, ValueChangedEventArgs args)
        {
            SpeedSlider.Value = Math.Round(args.NewValue);

            if (Math.Abs(SpeedSlider.Value - Speed) >= 1)
            {
                Speed = (int)SpeedSlider.Value;
                SpeedLabel.Text = $"Speed: {Speed}";

                TransmitDataToBluetoothDevice();
            }
        }

        private void OnMatrixBtnClicked(object sender, EventArgs args)
        {
            if (sender is Button button)
            {
                if (DrawingRBtn.IsChecked)
                {
                    int i = FirstMatrixBtn.BackgroundColor == Color.FromHex("#ff2400") ? 0 :
                        SecondMatrixBtn.BackgroundColor == Color.FromHex("#ff2400") ? 1 :
                        ThirdMatrixBtn.BackgroundColor == Color.FromHex("#ff2400") ? 2 : 3;

                    if (button.BackgroundColor == Color.FromHex("#203126"))
                        button.BackgroundColor = Color.FromHex("#ff3900");
                    else
                        button.BackgroundColor = Color.FromHex("#203126");

                    btnColors[i, Grid.GetRow(button), Grid.GetColumn(button)] = button.BackgroundColor;

                    TransmitDataToBluetoothDevice(TextOrMatrixChanged: true);
                }
            }
        }

        private void OnMatrixSelectionBtnClicked(object sender, EventArgs args)
        {
            if (sender is Button selectedButton)
            {
                if (DrawingRBtn.IsChecked)
                {
                    foreach (Button button in MatrixSelection.Children)
                    {
                        if (button != selectedButton)
                            button.BackgroundColor = Color.FromHex("#203126");
                    }
                    selectedButton.BackgroundColor = Color.FromHex("#ff2400");

                    int i = selectedButton == this.FindByName<Button>("FirstMatrixBtn") ? 0 :
                        selectedButton == this.FindByName<Button>("SecondMatrixBtn") ? 1 :
                        selectedButton == this.FindByName<Button>("ThirdMatrixBtn") ? 2 : 3;

                    foreach (Button button in DrawingGrid.Children)
                    {
                        button.BackgroundColor = btnColors[i, Grid.GetRow(button), Grid.GetColumn(button)];
                    }
                }
            }
        }

        private void ResetBtn_Clicked(object sender, EventArgs e)
        {
            const int ANY_MATRIX = 0;

            for (int i = 0; i < MATRIX_COUNT; i++)
                for (int j = 0; j < MATRIX_HEIGHT; j++)
                    for (int k = 0; k < MATRIX_WIDTH; k++)
                        btnColors[i, j, k] = Color.FromHex("#203126");


            foreach (Button button in DrawingGrid.Children)
            {
                button.BackgroundColor = btnColors[ANY_MATRIX, Grid.GetRow(button), Grid.GetColumn(button)];
            }

            TransmitDataToBluetoothDevice(TextOrMatrixChanged: true);
        }

        private async void TransmitDataToBluetoothDevice(bool TextOrMatrixChanged = false)
        {
            if (Application.Current.BindingContext is IBluetoothManagedConnection connection)
            {
                try
                {
                    List<byte> byteList = new List<byte>();
                    byteList.AddRange(new byte[]
                    {
                        Convert.ToByte(MatrixSwitch.IsToggled),
                        Convert.ToByte(TextRBtn.IsChecked),
                        Convert.ToByte(StaticModeSwitch.IsToggled),
                        Convert.ToByte(BrightnessSlider.Value),
                        Convert.ToByte(SpeedSlider.Value)
                    });

                    if (TextOrMatrixChanged)
                    {
                        // Если режим рисования, то последние передаваемые байты - состояния клеток матриц
                        if (DrawingRBtn.IsChecked)
                        {
                            List<byte> bitList = new List<byte>();

                            for (int i = 0; i < MATRIX_COUNT; i++)
                                for (int j = 0; j < MATRIX_WIDTH; j++)
                                    for (int k = MATRIX_HEIGHT - 1; k >= 0; k--)
                                        if (btnColors[i, k, j] == Color.FromHex("#203126"))
                                            bitList.Add(OFF);
                                        else
                                            bitList.Add(ON);

                            for (int i = 0; i < MATRIX_COUNT * MATRIX_WIDTH * MATRIX_HEIGHT; i += 8)
                                byteList.Add((byte)(bitList[i] << 7 | bitList[i + 1] << 6 | bitList[i + 2] << 5 | bitList[i + 3] << 4 | bitList[i + 4] << 3 | bitList[i + 5] << 2 | bitList[i + 6] << 1 | bitList[i + 7] << 0));
                        }
                        // Иначе последние передаваемые байты - текст
                        else
                        {
                            if (EntryText.Text != string.Empty)
                                byteList.AddRange(Encoding.GetEncoding(1251).GetBytes(EntryText.Text));
                        }
                    }

                    byteList.Insert(0, Convert.ToByte(2));
                    byteList.Add(Convert.ToByte(3));

                    byte[] buffer = byteList.ToArray();

                    connection.Transmit(buffer, OFFSET_DEFAULT, buffer.Length);
                }
                catch (Exception ex)
                {
                    await DisplayAlert("Error", ex.Message, "OK");
                }
            }
        }
    }
}