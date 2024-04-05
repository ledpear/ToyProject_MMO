namespace ChatClient.NET6._0
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
            txtName = new TextBox();
            label1 = new Label();
            btnConnect = new Button();
            txtChatMsg = new TextBox();
            txtMsg = new TextBox();
            SuspendLayout();
            // 
            // txtName
            // 
            txtName.Location = new Point(63, 18);
            txtName.Name = "txtName";
            txtName.Size = new Size(248, 23);
            txtName.TabIndex = 0;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(12, 22);
            label1.Name = "label1";
            label1.Size = new Size(43, 15);
            label1.TabIndex = 1;
            label1.Text = "대화명";
            // 
            // btnConnect
            // 
            btnConnect.Location = new Point(326, 18);
            btnConnect.Name = "btnConnect";
            btnConnect.Size = new Size(75, 23);
            btnConnect.TabIndex = 2;
            btnConnect.Text = "입장";
            btnConnect.UseVisualStyleBackColor = true;
            btnConnect.Click += btnConnect_Click;
            // 
            // txtChatMsg
            // 
            txtChatMsg.Location = new Point(12, 56);
            txtChatMsg.Multiline = true;
            txtChatMsg.Name = "txtChatMsg";
            txtChatMsg.ScrollBars = ScrollBars.Vertical;
            txtChatMsg.Size = new Size(389, 417);
            txtChatMsg.TabIndex = 3;
            // 
            // txtMsg
            // 
            txtMsg.Location = new Point(12, 492);
            txtMsg.Name = "txtMsg";
            txtMsg.Size = new Size(389, 23);
            txtMsg.TabIndex = 4;
            txtMsg.KeyPress += txtMsg_KeyPress;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(416, 527);
            Controls.Add(txtMsg);
            Controls.Add(txtChatMsg);
            Controls.Add(btnConnect);
            Controls.Add(label1);
            Controls.Add(txtName);
            Name = "Form1";
            Text = "Form1";
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private TextBox txtName;
        private Label label1;
        private Button btnConnect;
        private TextBox txtChatMsg;
        private TextBox txtMsg;
    }
}
