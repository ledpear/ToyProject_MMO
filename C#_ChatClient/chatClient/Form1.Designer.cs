namespace chatClient
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
            chatTextBox = new TextBox();
            sendTextBox = new TextBox();
            sendButton = new Button();
            enterButton = new Button();
            addressTextBox = new TextBox();
            SuspendLayout();
            // 
            // chatTextBox
            // 
            chatTextBox.Location = new Point(12, 60);
            chatTextBox.Multiline = true;
            chatTextBox.Name = "chatTextBox";
            chatTextBox.ReadOnly = true;
            chatTextBox.ScrollBars = ScrollBars.Vertical;
            chatTextBox.Size = new Size(414, 440);
            chatTextBox.TabIndex = 0;
            // 
            // sendTextBox
            // 
            sendTextBox.Location = new Point(12, 515);
            sendTextBox.Name = "sendTextBox";
            sendTextBox.Size = new Size(342, 23);
            sendTextBox.TabIndex = 1;
            sendTextBox.KeyPress += sendTextBox_KeyPress;
            // 
            // sendButton
            // 
            sendButton.Location = new Point(360, 515);
            sendButton.Name = "sendButton";
            sendButton.Size = new Size(66, 23);
            sendButton.TabIndex = 2;
            sendButton.Text = "전송";
            sendButton.UseVisualStyleBackColor = true;
            sendButton.Click += sendButton_Click;
            // 
            // enterButton
            // 
            enterButton.Location = new Point(351, 12);
            enterButton.Name = "enterButton";
            enterButton.Size = new Size(75, 23);
            enterButton.TabIndex = 3;
            enterButton.Text = "입장";
            enterButton.UseVisualStyleBackColor = true;
            enterButton.Click += enterButton_Click;
            // 
            // addressTextBox
            // 
            addressTextBox.Location = new Point(21, 16);
            addressTextBox.Name = "addressTextBox";
            addressTextBox.Size = new Size(196, 23);
            addressTextBox.TabIndex = 4;
            addressTextBox.Text = "127.0.0.1";
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(438, 558);
            Controls.Add(addressTextBox);
            Controls.Add(enterButton);
            Controls.Add(sendButton);
            Controls.Add(sendTextBox);
            Controls.Add(chatTextBox);
            Name = "Form1";
            Text = "ChatClient";
            FormClosed += Form1_FormClosed;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private TextBox chatTextBox;
        private TextBox sendTextBox;
        private Button sendButton;
        private Button enterButton;
        private TextBox addressTextBox;
    }
}
