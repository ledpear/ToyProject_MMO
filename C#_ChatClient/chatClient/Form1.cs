using System.Net;
using System.Net.Sockets;
using System.Text;


namespace chatClient
{
    public partial class Form1 : Form
    {
        private bool isConnect = false;
        private TcpClient tcpClient = null;
        private NetworkStream networkStream = null;
        private ChatHandler chatHandler = new ChatHandler();

        private delegate void SetTextDelegate(string text);

        public Form1()
        {
            InitializeComponent();
        }

        private void enterButton_Click(object sender, EventArgs e)
        {
            if (isConnect == false)
            {
                try
                {
                    tcpClient = new TcpClient();
                    tcpClient.Connect(IPAddress.Parse(addressTextBox.Text), 12000);
                    networkStream = tcpClient.GetStream();

                    chatHandler.Setup(this, networkStream, this.chatTextBox);
                    Thread chatThread = new Thread(new ThreadStart(chatHandler.ChatProcess));
                    chatThread.Start();
                    startChat();
                }
                catch (System.Exception Err)
                {
                    MessageBox.Show("ä�� ������ ���� �� ������ �߻��Ͽ����ϴ�. (Error : " + Err.Message + ")", "Client");
                }
            }
            else
            {
                CloseChat();
            }
        }

        public void SetText(string text)
        {
            //ũ�ν� ������ ���� �ذ�
            if (this.sendTextBox.InvokeRequired)
            {
                SetTextDelegate setTextDelegate = new SetTextDelegate(SetText);
                this.Invoke(setTextDelegate, new object[] { text });
            }
            else
            {
                this.chatTextBox.AppendText(text);
            }
        }

        private void sendMessage(string message, Boolean Msg)
        {
            try
            {
                string dataToSend = message + "\r\n";
                byte[] sendData = Encoding.Default.GetBytes(message);
                networkStream.Write(sendData, 0, sendData.Length);
            }
            catch (Exception Ex)
            {
                if (Msg == true)
                {
                    MessageBox.Show("������ Start ���� �ʾҰų�\n\n" + Ex.Message, "Client");
                    CloseChat();
                }
            }
        }

        private void startChat()
        {
            isConnect = true;
            this.enterButton.Text = "����";
            SetText("����\r\n");

        }

        private void CloseChat()
        {
            isConnect = false;
            this.enterButton.Text = "����";
            SetText("����\r\n");
            CloseChatClient();
        }

        private void CloseChatClient()
        {
            chatHandler.ChatClose();
            networkStream.Close();
            tcpClient.Close();
        }

        private void sendTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 13)
            {
                SendTextBoxMessage(sendTextBox.Text);
                e.Handled = true;
            }
        }

        private void SendTextBoxMessage(string text)
        {
            sendTextBox.Text = "";

            if (isConnect == false)
                return;

            sendMessage(text, true);
        }

        private void sendButton_Click(object sender, EventArgs e)
        {
            SendTextBoxMessage(sendTextBox.Text);
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            CloseChatClient();
            Application.Exit();
        }
    }

    public class ChatHandler
    {
        private Form1 form1;
        private NetworkStream networkStream;
        private StreamReader streamReader;
        private TextBox chatTextBox;

        public void Setup(Form1 form1, NetworkStream networkStream, TextBox chatTextBox)
        {
            this.chatTextBox = chatTextBox;
            this.networkStream = networkStream;
            this.form1 = form1;
            this.streamReader = new StreamReader(networkStream);
        }

        public void ChatProcess()
        {
            while(true)
            {
                try
                {
                    string message = streamReader.ReadLine();
                    if(message != null && message != "")
                        form1.SetText(message + "\r\n");
                }
                catch(System.Exception)
                {
                    break;
                }
            }
        }

        public void ChatClose()
        {
            streamReader.Close();
        }
    }
}
