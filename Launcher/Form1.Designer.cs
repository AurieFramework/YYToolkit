
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
            this.btOpenGitHub = new System.Windows.Forms.Button();
            this.btOpenPluginFolder = new System.Windows.Forms.Button();
            this.btYYTKLaunch = new System.Windows.Forms.Button();
            this.gbInject = new System.Windows.Forms.GroupBox();
            this.btOpenData = new System.Windows.Forms.Button();
            this.btNoModLaunch = new System.Windows.Forms.Button();
            this.rbStable = new System.Windows.Forms.RadioButton();
            this.rbBeta = new System.Windows.Forms.RadioButton();
            this.gbSourceCode = new System.Windows.Forms.GroupBox();
            this.btDownloadSrc = new System.Windows.Forms.Button();
            this.gbPlugins = new System.Windows.Forms.GroupBox();
            this.listPlugins = new System.Windows.Forms.ListBox();
            this.gbSelectFiles.SuspendLayout();
            this.gbMisc.SuspendLayout();
            this.gbInject.SuspendLayout();
            this.gbSourceCode.SuspendLayout();
            this.gbPlugins.SuspendLayout();
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
            this.txtDataFile.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(50)))));
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
            this.txtRunner.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(50)))));
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
            this.gbMisc.Controls.Add(this.btOpenGitHub);
            this.gbMisc.Controls.Add(this.btOpenPluginFolder);
            this.gbMisc.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gbMisc.ForeColor = System.Drawing.Color.White;
            this.gbMisc.Location = new System.Drawing.Point(12, 292);
            this.gbMisc.Name = "gbMisc";
            this.gbMisc.Size = new System.Drawing.Size(366, 69);
            this.gbMisc.TabIndex = 2;
            this.gbMisc.TabStop = false;
            this.gbMisc.Text = "4 - Miscellaneous";
            // 
            // btOpenGitHub
            // 
            this.btOpenGitHub.ForeColor = System.Drawing.Color.Black;
            this.btOpenGitHub.Location = new System.Drawing.Point(186, 26);
            this.btOpenGitHub.Name = "btOpenGitHub";
            this.btOpenGitHub.Size = new System.Drawing.Size(170, 28);
            this.btOpenGitHub.TabIndex = 1;
            this.btOpenGitHub.Text = "Open YYTK GitHub";
            this.btOpenGitHub.UseVisualStyleBackColor = true;
            this.btOpenGitHub.Click += new System.EventHandler(this.btOpenGitHub_Click);
            // 
            // btOpenPluginFolder
            // 
            this.btOpenPluginFolder.ForeColor = System.Drawing.Color.Black;
            this.btOpenPluginFolder.Location = new System.Drawing.Point(10, 26);
            this.btOpenPluginFolder.Name = "btOpenPluginFolder";
            this.btOpenPluginFolder.Size = new System.Drawing.Size(170, 28);
            this.btOpenPluginFolder.TabIndex = 0;
            this.btOpenPluginFolder.Text = "Open \'autoexec\' folder";
            this.btOpenPluginFolder.UseVisualStyleBackColor = true;
            this.btOpenPluginFolder.Click += new System.EventHandler(this.btOpenPluginFolder_Click);
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
            this.gbInject.Size = new System.Drawing.Size(268, 144);
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
            this.btOpenData.Text = "Open data.win in UTModTool";
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
            // rbStable
            // 
            this.rbStable.AutoSize = true;
            this.rbStable.Checked = true;
            this.rbStable.Location = new System.Drawing.Point(8, 33);
            this.rbStable.Name = "rbStable";
            this.rbStable.Size = new System.Drawing.Size(122, 25);
            this.rbStable.TabIndex = 2;
            this.rbStable.TabStop = true;
            this.rbStable.Text = "Stable Branch";
            this.rbStable.UseVisualStyleBackColor = true;
            // 
            // rbBeta
            // 
            this.rbBeta.AutoSize = true;
            this.rbBeta.Location = new System.Drawing.Point(8, 64);
            this.rbBeta.Name = "rbBeta";
            this.rbBeta.Size = new System.Drawing.Size(110, 25);
            this.rbBeta.TabIndex = 3;
            this.rbBeta.Text = "Beta Branch";
            this.rbBeta.UseVisualStyleBackColor = true;
            // 
            // gbSourceCode
            // 
            this.gbSourceCode.Controls.Add(this.btDownloadSrc);
            this.gbSourceCode.Controls.Add(this.rbStable);
            this.gbSourceCode.Controls.Add(this.rbBeta);
            this.gbSourceCode.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gbSourceCode.ForeColor = System.Drawing.Color.White;
            this.gbSourceCode.Location = new System.Drawing.Point(384, 259);
            this.gbSourceCode.Name = "gbSourceCode";
            this.gbSourceCode.Size = new System.Drawing.Size(268, 102);
            this.gbSourceCode.TabIndex = 4;
            this.gbSourceCode.TabStop = false;
            this.gbSourceCode.Text = "5 - Source Code";
            // 
            // btDownloadSrc
            // 
            this.btDownloadSrc.Font = new System.Drawing.Font("Segoe UI", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btDownloadSrc.ForeColor = System.Drawing.Color.Black;
            this.btDownloadSrc.Location = new System.Drawing.Point(136, 33);
            this.btDownloadSrc.Name = "btDownloadSrc";
            this.btDownloadSrc.Size = new System.Drawing.Size(126, 56);
            this.btDownloadSrc.TabIndex = 4;
            this.btDownloadSrc.Text = "Download";
            this.btDownloadSrc.UseVisualStyleBackColor = true;
            this.btDownloadSrc.Click += new System.EventHandler(this.btDownloadSrc_Click);
            // 
            // gbPlugins
            // 
            this.gbPlugins.Controls.Add(this.listPlugins);
            this.gbPlugins.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gbPlugins.ForeColor = System.Drawing.Color.White;
            this.gbPlugins.Location = new System.Drawing.Point(12, 109);
            this.gbPlugins.Name = "gbPlugins";
            this.gbPlugins.Size = new System.Drawing.Size(366, 177);
            this.gbPlugins.TabIndex = 5;
            this.gbPlugins.TabStop = false;
            this.gbPlugins.Text = "2 - Plugin Manager";
            // 
            // listPlugins
            // 
            this.listPlugins.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(32)))), ((int)(((byte)(32)))), ((int)(((byte)(32)))));
            this.listPlugins.ForeColor = System.Drawing.Color.White;
            this.listPlugins.FormattingEnabled = true;
            this.listPlugins.ItemHeight = 21;
            this.listPlugins.Location = new System.Drawing.Point(10, 31);
            this.listPlugins.Name = "listPlugins";
            this.listPlugins.Size = new System.Drawing.Size(346, 130);
            this.listPlugins.TabIndex = 0;
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(70)))));
            this.ClientSize = new System.Drawing.Size(664, 370);
            this.Controls.Add(this.gbMisc);
            this.Controls.Add(this.gbPlugins);
            this.Controls.Add(this.gbSourceCode);
            this.Controls.Add(this.gbInject);
            this.Controls.Add(this.gbSelectFiles);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MainWindow";
            this.Text = "YYLauncher 0.0.6c";
            this.gbSelectFiles.ResumeLayout(false);
            this.gbSelectFiles.PerformLayout();
            this.gbMisc.ResumeLayout(false);
            this.gbInject.ResumeLayout(false);
            this.gbSourceCode.ResumeLayout(false);
            this.gbSourceCode.PerformLayout();
            this.gbPlugins.ResumeLayout(false);
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
        private System.Windows.Forms.Button btOpenPluginFolder;
        private System.Windows.Forms.Button btOpenGitHub;
        private System.Windows.Forms.RadioButton rbStable;
        private System.Windows.Forms.RadioButton rbBeta;
        private System.Windows.Forms.GroupBox gbSourceCode;
        private System.Windows.Forms.Button btDownloadSrc;
        private System.Windows.Forms.GroupBox gbPlugins;
        private System.Windows.Forms.ListBox listPlugins;
        private System.Windows.Forms.Button btOpenData;
    }
}

