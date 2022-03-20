
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
            this.gbMisc = new System.Windows.Forms.GroupBox();
            this.btOpenUMTGitHub = new System.Windows.Forms.Button();
            this.btOpenGitHub = new System.Windows.Forms.Button();
            this.btYYTKLaunch = new System.Windows.Forms.Button();
            this.gbInject = new System.Windows.Forms.GroupBox();
            this.btOpenData = new System.Windows.Forms.Button();
            this.btNoModLaunch = new System.Windows.Forms.Button();
            this.gbPlugins = new System.Windows.Forms.GroupBox();
            this.listPlugins = new System.Windows.Forms.ListBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.cbUsePreloading = new System.Windows.Forms.CheckBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.gbSelectFiles.SuspendLayout();
            this.gbMisc.SuspendLayout();
            this.gbInject.SuspendLayout();
            this.gbPlugins.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // btRunnerPick
            // 
            this.btRunnerPick.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btRunnerPick.ForeColor = System.Drawing.SystemColors.ControlText;
            this.btRunnerPick.Location = new System.Drawing.Point(444, 26);
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
            this.gbSelectFiles.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gbSelectFiles.ForeColor = System.Drawing.Color.White;
            this.gbSelectFiles.Location = new System.Drawing.Point(12, 12);
            this.gbSelectFiles.Name = "gbSelectFiles";
            this.gbSelectFiles.Size = new System.Drawing.Size(640, 91);
            this.gbSelectFiles.TabIndex = 1;
            this.gbSelectFiles.TabStop = false;
            this.gbSelectFiles.Text = "1 - Select Game Files";
            // 
            // btResetData
            // 
            this.btResetData.ForeColor = System.Drawing.Color.Black;
            this.btResetData.Location = new System.Drawing.Point(561, 55);
            this.btResetData.Name = "btResetData";
            this.btResetData.Size = new System.Drawing.Size(75, 28);
            this.btResetData.TabIndex = 7;
            this.btResetData.Text = "Reset";
            this.btResetData.UseVisualStyleBackColor = true;
            this.btResetData.Click += new System.EventHandler(this.btResetData_Click);
            // 
            // btDataPick
            // 
            this.btDataPick.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btDataPick.ForeColor = System.Drawing.SystemColors.ControlText;
            this.btDataPick.Location = new System.Drawing.Point(444, 55);
            this.btDataPick.Name = "btDataPick";
            this.btDataPick.Size = new System.Drawing.Size(111, 28);
            this.btDataPick.TabIndex = 6;
            this.btDataPick.Text = "Pick data.win";
            this.btDataPick.UseVisualStyleBackColor = true;
            this.btDataPick.Click += new System.EventHandler(this.btDataPick_Click);
            // 
            // btResetRunner
            // 
            this.btResetRunner.ForeColor = System.Drawing.Color.Black;
            this.btResetRunner.Location = new System.Drawing.Point(561, 26);
            this.btResetRunner.Name = "btResetRunner";
            this.btResetRunner.Size = new System.Drawing.Size(75, 28);
            this.btResetRunner.TabIndex = 5;
            this.btResetRunner.Text = "Reset";
            this.btResetRunner.UseVisualStyleBackColor = true;
            this.btResetRunner.Click += new System.EventHandler(this.btResetRunner_Click);
            // 
            // txtDataFile
            // 
            this.txtDataFile.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(6)))), ((int)(((byte)(22)))), ((int)(((byte)(45)))));
            this.txtDataFile.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtDataFile.ForeColor = System.Drawing.Color.Silver;
            this.txtDataFile.Location = new System.Drawing.Point(91, 55);
            this.txtDataFile.Name = "txtDataFile";
            this.txtDataFile.ReadOnly = true;
            this.txtDataFile.Size = new System.Drawing.Size(347, 29);
            this.txtDataFile.TabIndex = 4;
            this.txtDataFile.Text = "<use default>";
            // 
            // lbDataFile
            // 
            this.lbDataFile.AutoSize = true;
            this.lbDataFile.Location = new System.Drawing.Point(6, 59);
            this.lbDataFile.Name = "lbDataFile";
            this.lbDataFile.Size = new System.Drawing.Size(68, 20);
            this.lbDataFile.TabIndex = 3;
            this.lbDataFile.Text = "Data File";
            // 
            // lbRunner
            // 
            this.lbRunner.AutoSize = true;
            this.lbRunner.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbRunner.Location = new System.Drawing.Point(6, 28);
            this.lbRunner.Name = "lbRunner";
            this.lbRunner.Size = new System.Drawing.Size(61, 21);
            this.lbRunner.TabIndex = 2;
            this.lbRunner.Text = "Runner";
            // 
            // txtRunner
            // 
            this.txtRunner.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(6)))), ((int)(((byte)(22)))), ((int)(((byte)(45)))));
            this.txtRunner.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtRunner.ForeColor = System.Drawing.Color.Silver;
            this.txtRunner.Location = new System.Drawing.Point(91, 25);
            this.txtRunner.Name = "txtRunner";
            this.txtRunner.ReadOnly = true;
            this.txtRunner.Size = new System.Drawing.Size(347, 29);
            this.txtRunner.TabIndex = 1;
            this.txtRunner.Text = "<none selected>";
            // 
            // gbMisc
            // 
            this.gbMisc.Controls.Add(this.btOpenUMTGitHub);
            this.gbMisc.Controls.Add(this.btOpenGitHub);
            this.gbMisc.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gbMisc.ForeColor = System.Drawing.Color.White;
            this.gbMisc.Location = new System.Drawing.Point(384, 251);
            this.gbMisc.Name = "gbMisc";
            this.gbMisc.Size = new System.Drawing.Size(268, 96);
            this.gbMisc.TabIndex = 2;
            this.gbMisc.TabStop = false;
            this.gbMisc.Text = "4 - Quick Repo Access";
            // 
            // btOpenUMTGitHub
            // 
            this.btOpenUMTGitHub.ForeColor = System.Drawing.Color.Black;
            this.btOpenUMTGitHub.Location = new System.Drawing.Point(8, 60);
            this.btOpenUMTGitHub.Name = "btOpenUMTGitHub";
            this.btOpenUMTGitHub.Size = new System.Drawing.Size(252, 28);
            this.btOpenUMTGitHub.TabIndex = 2;
            this.btOpenUMTGitHub.Text = "Open UndertaleModTool releases";
            this.btOpenUMTGitHub.UseVisualStyleBackColor = true;
            this.btOpenUMTGitHub.Click += new System.EventHandler(this.btOpenUMTGitHub_Click);
            // 
            // btOpenGitHub
            // 
            this.btOpenGitHub.ForeColor = System.Drawing.Color.Black;
            this.btOpenGitHub.Location = new System.Drawing.Point(8, 26);
            this.btOpenGitHub.Name = "btOpenGitHub";
            this.btOpenGitHub.Size = new System.Drawing.Size(252, 28);
            this.btOpenGitHub.TabIndex = 1;
            this.btOpenGitHub.Text = "Open YYToolkit releases";
            this.btOpenGitHub.UseVisualStyleBackColor = true;
            this.btOpenGitHub.Click += new System.EventHandler(this.btOpenGitHub_Click);
            // 
            // btYYTKLaunch
            // 
            this.btYYTKLaunch.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btYYTKLaunch.ForeColor = System.Drawing.Color.Black;
            this.btYYTKLaunch.Location = new System.Drawing.Point(8, 28);
            this.btYYTKLaunch.Name = "btYYTKLaunch";
            this.btYYTKLaunch.Size = new System.Drawing.Size(254, 28);
            this.btYYTKLaunch.TabIndex = 3;
            this.btYYTKLaunch.Text = "Launch game with YYToolkit";
            this.btYYTKLaunch.UseVisualStyleBackColor = true;
            this.btYYTKLaunch.Click += new System.EventHandler(this.btYYTKLaunch_Click);
            // 
            // gbInject
            // 
            this.gbInject.Controls.Add(this.btOpenData);
            this.gbInject.Controls.Add(this.btNoModLaunch);
            this.gbInject.Controls.Add(this.btYYTKLaunch);
            this.gbInject.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gbInject.ForeColor = System.Drawing.Color.White;
            this.gbInject.Location = new System.Drawing.Point(384, 109);
            this.gbInject.Name = "gbInject";
            this.gbInject.Size = new System.Drawing.Size(268, 136);
            this.gbInject.TabIndex = 4;
            this.gbInject.TabStop = false;
            this.gbInject.Text = "3 - Launcher";
            // 
            // btOpenData
            // 
            this.btOpenData.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btOpenData.ForeColor = System.Drawing.Color.Black;
            this.btOpenData.Location = new System.Drawing.Point(8, 96);
            this.btOpenData.Name = "btOpenData";
            this.btOpenData.Size = new System.Drawing.Size(254, 28);
            this.btOpenData.TabIndex = 6;
            this.btOpenData.Text = "Open in UndertaleModTool";
            this.btOpenData.UseVisualStyleBackColor = true;
            this.btOpenData.Click += new System.EventHandler(this.btOpenData_Click);
            // 
            // btNoModLaunch
            // 
            this.btNoModLaunch.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btNoModLaunch.ForeColor = System.Drawing.Color.Black;
            this.btNoModLaunch.Location = new System.Drawing.Point(8, 62);
            this.btNoModLaunch.Name = "btNoModLaunch";
            this.btNoModLaunch.Size = new System.Drawing.Size(254, 28);
            this.btNoModLaunch.TabIndex = 5;
            this.btNoModLaunch.Text = "Launch game without mods";
            this.btNoModLaunch.UseVisualStyleBackColor = true;
            this.btNoModLaunch.Click += new System.EventHandler(this.btNoModLaunch_Click);
            // 
            // gbPlugins
            // 
            this.gbPlugins.Controls.Add(this.listPlugins);
            this.gbPlugins.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gbPlugins.ForeColor = System.Drawing.Color.White;
            this.gbPlugins.Location = new System.Drawing.Point(12, 109);
            this.gbPlugins.Name = "gbPlugins";
            this.gbPlugins.Size = new System.Drawing.Size(366, 302);
            this.gbPlugins.TabIndex = 5;
            this.gbPlugins.TabStop = false;
            this.gbPlugins.Text = "2 - Plugin Manager";
            // 
            // listPlugins
            // 
            this.listPlugins.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(6)))), ((int)(((byte)(22)))), ((int)(((byte)(45)))));
            this.listPlugins.ForeColor = System.Drawing.Color.White;
            this.listPlugins.FormattingEnabled = true;
            this.listPlugins.ItemHeight = 21;
            this.listPlugins.Location = new System.Drawing.Point(10, 31);
            this.listPlugins.Name = "listPlugins";
            this.listPlugins.Size = new System.Drawing.Size(346, 256);
            this.listPlugins.TabIndex = 0;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.cbUsePreloading);
            this.groupBox1.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox1.ForeColor = System.Drawing.Color.White;
            this.groupBox1.Location = new System.Drawing.Point(384, 353);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(268, 58);
            this.groupBox1.TabIndex = 6;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "5 - Settings";
            // 
            // cbUsePreloading
            // 
            this.cbUsePreloading.AutoSize = true;
            this.cbUsePreloading.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(3)))), ((int)(((byte)(11)))), ((int)(((byte)(30)))));
            this.cbUsePreloading.Location = new System.Drawing.Point(8, 26);
            this.cbUsePreloading.Name = "cbUsePreloading";
            this.cbUsePreloading.Size = new System.Drawing.Size(149, 24);
            this.cbUsePreloading.TabIndex = 0;
            this.cbUsePreloading.Text = "Use early injection";
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
            this.ClientSize = new System.Drawing.Size(660, 423);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.gbMisc);
            this.Controls.Add(this.gbPlugins);
            this.Controls.Add(this.gbInject);
            this.Controls.Add(this.gbSelectFiles);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MainWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "YYToolkit Launcher (v0.1.1)";
            this.gbSelectFiles.ResumeLayout(false);
            this.gbSelectFiles.PerformLayout();
            this.gbMisc.ResumeLayout(false);
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
        private System.Windows.Forms.GroupBox gbMisc;
        private System.Windows.Forms.Button btYYTKLaunch;
        private System.Windows.Forms.GroupBox gbInject;
        private System.Windows.Forms.Button btNoModLaunch;
        private System.Windows.Forms.Button btResetRunner;
        private System.Windows.Forms.TextBox txtDataFile;
        private System.Windows.Forms.Label lbDataFile;
        private System.Windows.Forms.Button btResetData;
        private System.Windows.Forms.Button btDataPick;
        private System.Windows.Forms.Button btOpenGitHub;
        private System.Windows.Forms.GroupBox gbPlugins;
        private System.Windows.Forms.ListBox listPlugins;
        private System.Windows.Forms.Button btOpenData;
        private System.Windows.Forms.Button btOpenUMTGitHub;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.CheckBox cbUsePreloading;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}

