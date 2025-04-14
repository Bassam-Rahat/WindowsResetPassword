using Microsoft.Web.WebView2.WinForms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
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
            
            try
            {
                // Close existing WebView2 processes
                CleanupWebView2Processes();
                
                // Add a slight delay to allow processes to terminate
                System.Threading.Thread.Sleep(1000);
                
                // Use a custom user data folder to avoid conflicts
                InitializeComponentsWithCustomFolder();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error initializing application: {ex.Message}\n\nThe application will now close.", 
                    "Initialization Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Environment.Exit(1);
            }
        }

        private void CleanupWebView2Processes()
        {
            try
            {
                // Kill related processes that might be locking WebView2 resources
                string[] processNames = { "WebView2*", "msedgewebview2*", "Microsoft.Web.WebView2*", "msedge" };
                
                foreach (var process in Process.GetProcesses())
                {
                    try 
                    {
                        if (process.ProcessName.ToLower().Contains("webview2") || 
                            process.ProcessName.ToLower().Contains("msedge"))
                        {
                            process.Kill();
                        }
                    }
                    catch { /* Ignore if we can't kill this process */ }
                }
            }
            catch { /* Ignore errors if we can't kill processes */ }
        }

        private async void InitializeComponentsWithCustomFolder()
        {
            this.Text = "Reset Password";
            this.Width = 1024;
            this.Height = 768;

            try
            {
                // Create a unique user data folder
                string userDataFolder = Path.Combine(
                    Path.GetTempPath(), 
                    "WebViewLauncher_UserData_" + DateTime.Now.Ticks.ToString());
                
                // Ensure the directory exists
                if (!Directory.Exists(userDataFolder))
                {
                    Directory.CreateDirectory(userDataFolder);
                }

                webView = new WebView2
                {
                    Dock = DockStyle.Fill
                };

                this.Controls.Add(webView);

                // Create WebView2 environment options with the custom user data folder
                var options = new Microsoft.Web.WebView2.Core.CoreWebView2EnvironmentOptions();
                options.AdditionalBrowserArguments = "--no-sandbox --disable-web-security";
                
                var environment = await Microsoft.Web.WebView2.Core.CoreWebView2Environment.CreateAsync(
                    null, userDataFolder, options);

                // Initialize with custom environment
                await webView.EnsureCoreWebView2Async(environment);
                
                // Set browser options
                webView.CoreWebView2.Settings.AreDefaultContextMenusEnabled = true;
                webView.CoreWebView2.Settings.IsScriptEnabled = true;
                webView.CoreWebView2.Settings.AreDevToolsEnabled = true;
                
                // Navigate to the website
                webView.CoreWebView2.Navigate("https://103.22.140.225:4343/");
            }
            catch (System.Runtime.InteropServices.COMException ex) when (ex.HResult == unchecked((int)0x800700AA))
            {
                MessageBox.Show("WebView2 resources are in use by another process. Please restart your computer and try again.",
                    "Resource Conflict", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Environment.Exit(1);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error initializing WebView2: {ex.Message}", 
                    "WebView Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Environment.Exit(1);
            }
        }

        private void BrowserForm_Load(object sender, EventArgs e)
        {
            // Additional initialization can be done here
        }
        
        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            base.OnFormClosing(e);
            
            try
            {
                // Cleanup WebView resources when closing
                if (webView != null)
                {
                    webView.Dispose();
                    webView = null;
                    
                    // Force GC to clean up resources
                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                }
            }
            catch { /* Ignore errors during cleanup */ }
        }
    }
}
