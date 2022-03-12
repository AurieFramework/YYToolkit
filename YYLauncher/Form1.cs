using System;
using System.Net;
using System.Windows.Forms;

namespace YYLauncher
{
    public partial class WndLauncher : Form
    {
        public WndLauncher()
        {
            InitializeComponent();
        }

        private void WndLauncher_Load(object sender, EventArgs e)
        {
            // Fix for Windows 7 Downloads
            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12; 
        }
    }
}
