
namespace YYLauncher
{
    partial class WndLauncher
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.RunnerTextBox = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // RunnerTextBox
            // 
            this.RunnerTextBox.AllowDrop = true;
            this.RunnerTextBox.BackColor = System.Drawing.Color.Black;
            this.RunnerTextBox.Enabled = false;
            this.RunnerTextBox.ForeColor = System.Drawing.Color.White;
            this.RunnerTextBox.Location = new System.Drawing.Point(156, 12);
            this.RunnerTextBox.MaxLength = 256;
            this.RunnerTextBox.Name = "RunnerTextBox";
            this.RunnerTextBox.Size = new System.Drawing.Size(707, 29);
            this.RunnerTextBox.TabIndex = 0;
            this.RunnerTextBox.Text = "<unset>";
            this.RunnerTextBox.WordWrap = false;
            // 
            // WndLauncher
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 21F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(32)))), ((int)(((byte)(32)))), ((int)(((byte)(32)))));
            this.ClientSize = new System.Drawing.Size(875, 434);
            this.Controls.Add(this.RunnerTextBox);
            this.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.MaximizeBox = false;
            this.Name = "WndLauncher";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "YYToolkit Launcher";
            this.Load += new System.EventHandler(this.WndLauncher_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox RunnerTextBox;
    }
}

