
namespace Launcher
{
    partial class MainWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }


        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindow));
            this.btRunnerPick = new System.Windows.Forms.Button();
            this.gbSelectFiles = new System.Windows.Forms.GroupBox();
            this.btResetData = new System.Windows.Forms.Button();
            this.btDataPick = new System.Windows.Forms.Button();
            this.btResetRunner = new System.Windows.Forms.Button();
            this.txtDataFile = new System.Windows.Forms.TextBox();
            this.lbDataFile = new System.Windows.Forms.Label();
            this.lbRunner = new System.Windows.Forms.Label();
            this.txtRunner = new System.Windows.Forms.TextBox();
            this.btYYTKLaunch = new System.Windows.Forms.Button();
            this.gbInject = new System.Windows.Forms.GroupBox();
            this.btLaunchCustomDLL = new System.Windows.Forms.Button();
            this.btOpenData = new System.Windows.Forms.Button();
            this.gbPlugins = new System.Windows.Forms.GroupBox();
            this.listPlugins = new System.Windows.Forms.ListBox();
            this.ctxStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.cbAutoCleanup = new System.Windows.Forms.CheckBox();
            this.cbUseRandomFilename = new System.Windows.Forms.CheckBox();
            this.cbForceUpdates = new System.Windows.Forms.CheckBox();
            this.cbUsePreloading = new System.Windows.Forms.CheckBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.gbSelectFiles.SuspendLayout();
            this.gbInject.SuspendLayout();
            this.gbPlugins.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // btRunnerPick
            // 
            this.btRunnerPick.Font = new System.Drawing.Font("Segoe UI", 11.25F);
            this.btRunnerPick.ForeColor = System.Drawing.SystemColors.ControlText;
            this.btRunnerPick.Location = new System.Drawing.Point(484, 25);
            this.btRunnerPick.Name = "btRunnerPick";
            this.btRunnerPick.Size = new System.Drawing.Size(111, 28);
            this.btRunnerPick.TabIndex = 0;
            this.btRunnerPick.Text = "Pick runner";
            this.btRunnerPick.UseVisualStyleBackColor = true;
            this.btRunnerPick.Click += new System.EventHandler(this.btRunnerPick_Click);
            // 
            // gbSelectFiles
            // 
            this.gbSelectFiles.Controls.Add(this.btResetData);
            this.gbSelectFiles.Controls.Add(this.btDataPick);
            this.gbSelectFiles.Controls.Add(this.btResetRunner);
            this.gbSelectFiles.Controls.Add(this.txtDataFile);
            this.gbSelectFiles.Controls.Add(this.lbDataFile);
            this.gbSelectFiles.Controls.Add(this.lbRunner);
            this.gbSelectFiles.Controls.Add(this.txtRunner);
            this.gbSelectFiles.Controls.Add(this.btRunnerPick);
            this.gbSelectFiles.Font = new System.Drawing.Font("Segoe UI Semibold", 11.25F);
            this.gbSelectFiles.ForeColor = System.Drawing.Color.White;
            this.gbSelectFiles.Location = new System.Drawing.Point(12, 12);
            this.gbSelectFiles.Name = "gbSelectFiles";
            this.gbSelectFiles.Size = new System.Drawing.Size(682, 94);
            this.gbSelectFiles.TabIndex = 1;
            this.gbSelectFiles.TabStop = false;
            this.gbSelectFiles.Text = "Game Files";
            // 
            // btResetData
            // 
            this.btResetData.Font = new System.Drawing.Font("Segoe UI", 11.25F);
            this.btResetData.ForeColor = System.Drawing.Color.Black;
            this.btResetData.Location = new System.Drawing.Point(601, 55);
            this.btResetData.Name = "btResetData";
            this.btResetData.Size = new System.Drawing.Size(75, 29);
            this.btResetData.TabIndex = 7;
            this.btResetData.Text = "Reset";
            this.btResetData.UseVisualStyleBackColor = true;
            this.btResetData.Click += new System.EventHandler(this.btResetData_Click);
            // 
            // btDataPick
            // 
            this.btDataPick.Font = new System.Drawing.Font("Segoe UI", 11.25F);
            this.btDataPick.ForeColor = System.Drawing.SystemColors.ControlText;
            this.btDataPick.Location = new System.Drawing.Point(484, 55);
            this.btDataPick.Name = "btDataPick";
            this.btDataPick.Size = new System.Drawing.Size(111, 29);
            this.btDataPick.TabIndex = 6;
            this.btDataPick.Text = "Pick data.win";
            this.btDataPick.UseVisualStyleBackColor = true;
            this.btDataPick.Click += new System.EventHandler(this.btDataPick_Click);
            // 
            // btResetRunner
            // 
            this.btResetRunner.Font = new System.Drawing.Font("Segoe UI", 11.25F);
            this.btResetRunner.ForeColor = System.Drawing.Color.Black;
            this.btResetRunner.Location = new System.Drawing.Point(601, 25);
            this.btResetRunner.Name = "btResetRunner";
            this.btResetRunner.Size = new System.Drawing.Size(75, 29);
            this.btResetRunner.TabIndex = 5;
            this.btResetRunner.Text = "Reset";
            this.btResetRunner.UseVisualStyleBackColor = true;
            this.btResetRunner.Click += new System.EventHandler(this.btResetRunner_Click);
            // 
            // txtDataFile
            // 
            this.txtDataFile.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(6)))), ((int)(((byte)(22)))), ((int)(((byte)(45)))));
            this.txtDataFile.Font = new System.Drawing.Font("Segoe UI", 12F);
            this.txtDataFile.ForeColor = System.Drawing.Color.Silver;
            this.txtDataFile.Location = new System.Drawing.Point(91, 55);
            this.txtDataFile.Name = "txtDataFile";
            this.txtDataFile.ReadOnly = true;
            this.txtDataFile.Size = new System.Drawing.Size(387, 29);
            this.txtDataFile.TabIndex = 4;
            this.txtDataFile.Text = "<use default>";
            // 
            // lbDataFile
            // 
            this.lbDataFile.AutoSize = true;
            this.lbDataFile.Font = new System.Drawing.Font("Segoe UI", 11.25F);
            this.lbDataFile.Location = new System.Drawing.Point(6, 59);
            this.lbDataFile.Name = "lbDataFile";
            this.lbDataFile.Size = new System.Drawing.Size(68, 20);
            this.lbDataFile.TabIndex = 3;
            this.lbDataFile.Text = "Data File";
            // 
            // lbRunner
            // 
            this.lbRunner.AutoSize = true;
            this.lbRunner.Font = new System.Drawing.Font("Segoe UI", 12F);
            this.lbRunner.Location = new System.Drawing.Point(6, 28);
            this.lbRunner.Name = "lbRunner";
            this.lbRunner.Size = new System.Drawing.Size(61, 21);
            this.lbRunner.TabIndex = 2;
            this.lbRunner.Text = "Runner";
            // 
            // txtRunner
            // 
            this.txtRunner.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(6)))), ((int)(((byte)(22)))), ((int)(((byte)(45)))));
            this.txtRunner.Font = new System.Drawing.Font("Segoe UI", 12F);
            this.txtRunner.ForeColor = System.Drawing.Color.Silver;
            this.txtRunner.Location = new System.Drawing.Point(91, 25);
            this.txtRunner.Name = "txtRunner";
            this.txtRunner.ReadOnly = true;
            this.txtRunner.Size = new System.Drawing.Size(387, 29);
            this.txtRunner.TabIndex = 1;
            this.txtRunner.Text = "<none selected>";
            // 
            // btYYTKLaunch
            // 
            this.btYYTKLaunch.Font = new System.Drawing.Font("Segoe UI", 11.25F);
            this.btYYTKLaunch.ForeColor = System.Drawing.Color.Black;
            this.btYYTKLaunch.Location = new System.Drawing.Point(9, 28);
            this.btYYTKLaunch.Name = "btYYTKLaunch";
            this.btYYTKLaunch.Size = new System.Drawing.Size(309, 28);
            this.btYYTKLaunch.TabIndex = 3;
            this.btYYTKLaunch.Text = "Launch with YYToolkit";
            this.btYYTKLaunch.UseVisualStyleBackColor = true;
            this.btYYTKLaunch.Click += new System.EventHandler(this.btYYTKLaunch_Click);
            // 
            // gbInject
            // 
            this.gbInject.Controls.Add(this.btLaunchCustomDLL);
            this.gbInject.Controls.Add(this.btOpenData);
            this.gbInject.Controls.Add(this.btYYTKLaunch);
            this.gbInject.Font = new System.Drawing.Font("Segoe UI Semibold", 12F);
            this.gbInject.ForeColor = System.Drawing.Color.White;
            this.gbInject.Location = new System.Drawing.Point(370, 198);
            this.gbInject.Name = "gbInject";
            this.gbInject.Size = new System.Drawing.Size(324, 134);
            this.gbInject.TabIndex = 4;
            this.gbInject.TabStop = false;
            this.gbInject.Text = "Launch options";
            // 
            // btLaunchCustomDLL
            // 
            this.btLaunchCustomDLL.Font = new System.Drawing.Font("Segoe UI", 11.25F);
            this.btLaunchCustomDLL.ForeColor = System.Drawing.Color.Black;
            this.btLaunchCustomDLL.Location = new System.Drawing.Point(9, 62);
            this.btLaunchCustomDLL.Name = "btLaunchCustomDLL";
            this.btLaunchCustomDLL.Size = new System.Drawing.Size(309, 28);
            this.btLaunchCustomDLL.TabIndex = 7;
            this.btLaunchCustomDLL.Text = "Launch with custom DLL";
            this.btLaunchCustomDLL.UseVisualStyleBackColor = true;
            this.btLaunchCustomDLL.Click += new System.EventHandler(this.btLaunchCustomDLL_Click);
            // 
            // btOpenData
            // 
            this.btOpenData.Font = new System.Drawing.Font("Segoe UI", 11.25F);
            this.btOpenData.ForeColor = System.Drawing.Color.Black;
            this.btOpenData.Location = new System.Drawing.Point(9, 96);
            this.btOpenData.Name = "btOpenData";
            this.btOpenData.Size = new System.Drawing.Size(309, 28);
            this.btOpenData.TabIndex = 6;
            this.btOpenData.Text = "Open game data with UndertaleModTool";
            this.btOpenData.UseVisualStyleBackColor = true;
            this.btOpenData.Click += new System.EventHandler(this.btOpenData_Click);
            // 
            // gbPlugins
            // 
            this.gbPlugins.Controls.Add(this.listPlugins);
            this.gbPlugins.Font = new System.Drawing.Font("Segoe UI Semibold", 12F);
            this.gbPlugins.ForeColor = System.Drawing.Color.White;
            this.gbPlugins.Location = new System.Drawing.Point(12, 112);
            this.gbPlugins.Name = "gbPlugins";
            this.gbPlugins.Size = new System.Drawing.Size(352, 220);
            this.gbPlugins.TabIndex = 5;
            this.gbPlugins.TabStop = false;
            this.gbPlugins.Text = "Plugin Manager";
            // 
            // listPlugins
            // 
            this.listPlugins.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(6)))), ((int)(((byte)(22)))), ((int)(((byte)(45)))));
            this.listPlugins.ContextMenuStrip = this.ctxStrip1;
            this.listPlugins.Font = new System.Drawing.Font("Segoe UI", 12F);
            this.listPlugins.ForeColor = System.Drawing.Color.White;
            this.listPlugins.FormattingEnabled = true;
            this.listPlugins.ItemHeight = 21;
            this.listPlugins.Location = new System.Drawing.Point(10, 31);
            this.listPlugins.Name = "listPlugins";
            this.listPlugins.Size = new System.Drawing.Size(332, 172);
            this.listPlugins.TabIndex = 0;
            this.toolTip1.SetToolTip(this.listPlugins, "A list of plugins installed for the current game.\r\nRight click for options.");
            // 
            // ctxStrip1
            // 
            this.ctxStrip1.Name = "ctxStrip1";
            this.ctxStrip1.Size = new System.Drawing.Size(61, 4);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.cbAutoCleanup);
            this.groupBox1.Controls.Add(this.cbUseRandomFilename);
            this.groupBox1.Controls.Add(this.cbForceUpdates);
            this.groupBox1.Controls.Add(this.cbUsePreloading);
            this.groupBox1.Font = new System.Drawing.Font("Segoe UI Semibold", 12F);
            this.groupBox1.ForeColor = System.Drawing.Color.White;
            this.groupBox1.Location = new System.Drawing.Point(370, 112);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(324, 88);
            this.groupBox1.TabIndex = 6;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Advanced Settings";
            // 
            // cbAutoCleanup
            // 
            this.cbAutoCleanup.AutoSize = true;
            this.cbAutoCleanup.Checked = true;
            this.cbAutoCleanup.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbAutoCleanup.Font = new System.Drawing.Font("Segoe UI", 12F);
            this.cbAutoCleanup.Location = new System.Drawing.Point(153, 55);
            this.cbAutoCleanup.Name = "cbAutoCleanup";
            this.cbAutoCleanup.Size = new System.Drawing.Size(122, 25);
            this.cbAutoCleanup.TabIndex = 3;
            this.cbAutoCleanup.Text = "Auto-cleanup";
            this.toolTip1.SetToolTip(this.cbAutoCleanup, resources.GetString("cbAutoCleanup.ToolTip"));
            this.cbAutoCleanup.UseVisualStyleBackColor = true;
            // 
            // cbUseRandomFilename
            // 
            this.cbUseRandomFilename.AutoSize = true;
            this.cbUseRandomFilename.Font = new System.Drawing.Font("Segoe UI", 12F);
            this.cbUseRandomFilename.Location = new System.Drawing.Point(153, 28);
            this.cbUseRandomFilename.Name = "cbUseRandomFilename";
            this.cbUseRandomFilename.Size = new System.Drawing.Size(165, 25);
            this.cbUseRandomFilename.TabIndex = 2;
            this.cbUseRandomFilename.Text = "Random DLL Name";
            this.toolTip1.SetToolTip(this.cbUseRandomFilename, "Uses a random filename for the YYToolkit DLL to avoid potential detection by game" +
        "s.");
            this.cbUseRandomFilename.UseVisualStyleBackColor = true;
            // 
            // cbForceUpdates
            // 
            this.cbForceUpdates.AutoSize = true;
            this.cbForceUpdates.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(3)))), ((int)(((byte)(11)))), ((int)(((byte)(30)))));
            this.cbForceUpdates.Checked = true;
            this.cbForceUpdates.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbForceUpdates.Font = new System.Drawing.Font("Segoe UI", 12F);
            this.cbForceUpdates.Location = new System.Drawing.Point(9, 28);
            this.cbForceUpdates.Name = "cbForceUpdates";
            this.cbForceUpdates.Size = new System.Drawing.Size(128, 25);
            this.cbForceUpdates.TabIndex = 1;
            this.cbForceUpdates.Text = "Force Updates";
            this.toolTip1.SetToolTip(this.cbForceUpdates, "Forces a redownload of the YYToolkit DLL upon game launch.\r\n\r\nIf you disable this" +
        ", you risk not having the latest YYToolkit release.");
            this.cbForceUpdates.UseVisualStyleBackColor = false;
            // 
            // cbUsePreloading
            // 
            this.cbUsePreloading.AutoSize = true;
            this.cbUsePreloading.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(3)))), ((int)(((byte)(11)))), ((int)(((byte)(30)))));
            this.cbUsePreloading.Font = new System.Drawing.Font("Segoe UI", 12F);
            this.cbUsePreloading.Location = new System.Drawing.Point(9, 55);
            this.cbUsePreloading.Name = "cbUsePreloading";
            this.cbUsePreloading.Size = new System.Drawing.Size(126, 25);
            this.cbUsePreloading.TabIndex = 0;
            this.cbUsePreloading.Text = "Early Injection";
            this.toolTip1.SetToolTip(this.cbUsePreloading, "Enables YYToolkit and plugins to run code before the game starts.\r\n\r\nIt is recomm" +
        "ended to leave this disabled, unless a plugin you\'re using\r\nspecifically require" +
        "s it to be enabled.");
            this.cbUsePreloading.UseVisualStyleBackColor = false;
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(3)))), ((int)(((byte)(11)))), ((int)(((byte)(30)))));
            this.ClientSize = new System.Drawing.Size(704, 341);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.gbPlugins);
            this.Controls.Add(this.gbInject);
            this.Controls.Add(this.gbSelectFiles);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(720, 380);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(720, 380);
            this.Name = "MainWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "YYToolkit Launcher (v2.1.0)";
            this.gbSelectFiles.ResumeLayout(false);
            this.gbSelectFiles.PerformLayout();
            this.gbInject.ResumeLayout(false);
            this.gbPlugins.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button btRunnerPick;
        private System.Windows.Forms.GroupBox gbSelectFiles;
        private System.Windows.Forms.Label lbRunner;
        private System.Windows.Forms.TextBox txtRunner;
        private System.Windows.Forms.Button btYYTKLaunch;
        private System.Windows.Forms.GroupBox gbInject;
        private System.Windows.Forms.Button btResetRunner;
        private System.Windows.Forms.TextBox txtDataFile;
        private System.Windows.Forms.Label lbDataFile;
        private System.Windows.Forms.Button btResetData;
        private System.Windows.Forms.Button btDataPick;
        private System.Windows.Forms.GroupBox gbPlugins;
        private System.Windows.Forms.ListBox listPlugins;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.CheckBox cbUsePreloading;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.Button btOpenData;
        private System.Windows.Forms.Button btLaunchCustomDLL;
        private System.Windows.Forms.ContextMenuStrip ctxStrip1;
        private System.Windows.Forms.CheckBox cbUseRandomFilename;
        private System.Windows.Forms.CheckBox cbAutoCleanup;
        private System.Windows.Forms.CheckBox cbForceUpdates;
    }
}