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
                    MessageBox.Show("채팅 서버와 연결 중 오류가 발생하였습니다. (Error : " + Err.Message + ")", "Client");
                }
            }
            else
            {
                CloseChat();
            }
        }

        public void SetText(string text)
        {
            //크로스 스레드 문제 해결
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
                    MessageBox.Show("서버가 Start 되지 않았거나\n\n" + Ex.Message, "Client");
                    CloseChat();
                }
            }
        }

        private void startChat()
        {
            isConnect = true;
            this.enterButton.Text = "퇴장";
            SetText("입장\r\n");

        }

        private void CloseChat()
        {
            isConnect = false;
            this.enterButton.Text = "입장";
            SetText("퇴장\r\n");
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
