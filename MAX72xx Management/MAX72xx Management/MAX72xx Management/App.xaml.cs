using System;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using Plugin.BluetoothClassic.Abstractions;

namespace MAX72xx_Management
{
    public partial class App : Application
    {
        public App()
        {
            InitializeComponent();

            MainPage = new MainPage();
        }

        protected override void OnStart()
        {
        }

        protected override void OnSleep()
        {
            if (Application.Current.BindingContext is IBluetoothConnection connection)
            {
                //Settings.lvBondedDevices.SelectedItem = null;
                connection.Dispose();
            }
        }

        protected override void OnResume()
        {
            //if (Application.Current.BindingContext is IBluetoothConnection connection)
            //{
                //connection = adapter.CreateConnection(device);
            //}
        }
    }
}
