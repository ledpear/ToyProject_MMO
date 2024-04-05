namespace ChatServer.NET6._0
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
            txtChatMsg = new TextBox();
            lblMsg = new Label();
            btnStart = new Button();
            SuspendLayout();
            // 
            // txtChatMsg
            // 
            txtChatMsg.Location = new Point(14, 18);
            txtChatMsg.Multiline = true;
            txtChatMsg.Name = "txtChatMsg";
            txtChatMsg.ScrollBars = ScrollBars.Vertical;
            txtChatMsg.Size = new Size(576, 316);
            txtChatMsg.TabIndex = 0;
            // 
            // lblMsg
            // 
            lblMsg.AutoSize = true;
            lblMsg.Font = new Font("맑은 고딕", 24F, FontStyle.Regular, GraphicsUnit.Point);
            lblMsg.Location = new Point(37, 368);
            lblMsg.Name = "lblMsg";
            lblMsg.Size = new Size(227, 45);
            lblMsg.TabIndex = 1;
            lblMsg.Tag = "Stop";
            lblMsg.Text = "Server 중지 됨";
            // 
            // btnStart
            // 
            btnStart.Font = new Font("맑은 고딕", 16F, FontStyle.Regular, GraphicsUnit.Point);
            btnStart.Location = new Point(416, 359);
            btnStart.Name = "btnStart";
            btnStart.Size = new Size(159, 64);
            btnStart.TabIndex = 2;
            btnStart.Text = "서버 시작";
            btnStart.UseVisualStyleBackColor = true;
            btnStart.Click += btnStart_Click;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(611, 450);
            Controls.Add(btnStart);
            Controls.Add(lblMsg);
            Controls.Add(txtChatMsg);
            Name = "Form1";
            Text = "Form1";
            FormClosed += Form1_FormClosed;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private TextBox txtChatMsg;
        private Label lblMsg;
        private Button btnStart;
    }
}
