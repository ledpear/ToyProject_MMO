namespace chatServer
{
    partial class Form1
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
            chatMessageTextBox = new TextBox();
            serverStartButton = new Button();
            serverStateLabel = new Label();
            SuspendLayout();
            // 
            // chatMessageTextBox
            // 
            chatMessageTextBox.Location = new Point(12, 12);
            chatMessageTextBox.Multiline = true;
            chatMessageTextBox.Name = "chatMessageTextBox";
            chatMessageTextBox.ScrollBars = ScrollBars.Vertical;
            chatMessageTextBox.Size = new Size(473, 330);
            chatMessageTextBox.TabIndex = 0;
            // 
            // serverStartButton
            // 
            serverStartButton.Font = new Font("맑은 고딕", 16F);
            serverStartButton.Location = new Point(321, 348);
            serverStartButton.Name = "serverStartButton";
            serverStartButton.Size = new Size(164, 90);
            serverStartButton.TabIndex = 1;
            serverStartButton.Text = "서버 시작";
            serverStartButton.UseVisualStyleBackColor = true;
            serverStartButton.Click += serverStartButton_Click;
            // 
            // serverStateLabel
            // 
            serverStateLabel.AutoSize = true;
            serverStateLabel.Font = new Font("맑은 고딕", 32F);
            serverStateLabel.Location = new Point(12, 367);
            serverStateLabel.Name = "serverStateLabel";
            serverStateLabel.Size = new Size(270, 59);
            serverStateLabel.TabIndex = 2;
            serverStateLabel.Text = "서버 중지 됨";
            serverStateLabel.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(497, 450);
            Controls.Add(serverStateLabel);
            Controls.Add(serverStartButton);
            Controls.Add(chatMessageTextBox);
            Name = "Form1";
            Text = "Form1";
            FormClosed += Form1_FormClosed;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private TextBox chatMessageTextBox;
        private Button serverStartButton;
        private Label serverStateLabel;
    }
}
