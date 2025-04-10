using Microsoft.Web.WebView2.WinForms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WebViewLauncher
{
    public partial class BrowserForm : Form
    {
        private WebView2 webView;
        public BrowserForm()
        {
            InitializeComponent();
            InitializeComponents();
        }

        private async void InitializeComponents()
        {
            this.Text = "Reset Password";
            this.Width = 1024;
            this.Height = 768;

            webView = new WebView2
            {
                Dock = DockStyle.Fill
            };

            this.Controls.Add(webView);

            await webView.EnsureCoreWebView2Async(null);
            webView.CoreWebView2.Navigate("https://103.22.140.225:4343/");
        }

        private void BrowserForm_Load(object sender, EventArgs e)
        {

        }
    }
}
