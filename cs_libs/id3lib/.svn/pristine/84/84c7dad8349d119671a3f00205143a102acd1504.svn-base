using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace TagEditor
{
    class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {

            UpgradeUserSettings();

            Application.Run(new MainForm());
        }

        private static void UpgradeUserSettings()
        {
            // hook up to user.config from previous build
            System.Reflection.Assembly a = System.Reflection.Assembly.GetExecutingAssembly();
            Version appVersion = a.GetName().Version;
            string appVersionString = appVersion.ToString();

            if (Properties.Settings.Default.ApplicationVersion != appVersion.ToString())
            {
                Properties.Settings.Default.Upgrade();
                Properties.Settings.Default.ApplicationVersion = appVersionString;
            }
        }
    }
}
