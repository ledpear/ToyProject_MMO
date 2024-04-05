using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ChatClient.NET6._0
{
    public partial class Form1 : Form
    {
        delegate void SetTextDelegate(string s);

        public Form1()
        {
            InitializeComponent();
        }

        TcpClient tcpClient = null;
        NetworkStream ntwStream = null;
        ChatHandler chatHandler = new ChatHandler();

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (btnConnect.Text == "입장")
            {
                try
                {
                    tcpClient = new TcpClient();
                    tcpClient.Connect(IPAddress.Parse("127.0.0.1"), 12000);
                    ntwStream = tcpClient.GetStream();

                    chatHandler.Setup(this, ntwStream, this.txtChatMsg);
                    Thread chatThread = new Thread(new ThreadStart(chatHandler.ChatProcess));
                    chatThread.Start();

                    Message_Snd("<" + txtName.Text + "> 님께서 접속 하셨습니다.", true);
                    btnConnect.Text = "나가기";
                }
                catch (System.Exception Ex)
                {
                    MessageBox.Show("오류발생 : " + Ex.ToString());
                }
            }
            else
            {
                Message_Snd("<" + txtName.Text + "> 님께서 접속해제 하셨습니다.", true);
                btnConnect.Text = "입장";
                chatHandler.ChatClose();
                ntwStream.Close();
                tcpClient.Close();

            }
        }

        private void Message_Snd(string lstMessage, Boolean Msg)
        {
            try
            {
                string dataToSend = lstMessage + "\r\n";
                byte[] data = Encoding.Default.GetBytes(dataToSend);
                ntwStream.Write(data, 0, data.Length);
            }
            catch (Exception ex)
            {
                if (Msg == true)
                {
                    MessageBox.Show("서버가 Start되지 않았거나\r\n" + ex.ToString());
                    btnConnect.Text = "입장";
                    chatHandler.ChatClose();
                    ntwStream.Close();
                    tcpClient.Close();
                }
            }
        }

        public void SetText(string text)
        {
            if (this.txtChatMsg.InvokeRequired)
            {
                SetTextDelegate d = new SetTextDelegate(SetText);
                this.Invoke(d, new object[] { text });
            }
            else
            {
                this.txtChatMsg.AppendText(text);
            }
        }

        private void txtMsg_KeyPress(object sender, KeyPressEventArgs e)
        {
            if(e.KeyChar == 13)
            {
                if(btnConnect.Text == "나가기")
                {
                    Message_Snd("<" + txtName.Text + "> " + txtMsg.Text, true);
                }

                txtMsg.Text = "";
                e.Handled = true;
            }
        }
    }

    public class ChatHandler
    {
        private TextBox txtChatMsg;
        private NetworkStream netStream;
        private StreamReader strReader;
        private Form1 form1;

        public void Setup(Form1 form1, NetworkStream netStream, TextBox txtChatMsg)
        {
            this.txtChatMsg = txtChatMsg;
            this.netStream = netStream;
            this.form1 = form1;
            this.netStream = netStream;
            this.strReader = new StreamReader(netStream);
        }

        public void ChatClose()
        {
            netStream.Close();
            strReader.Close();
        }

        public void ChatProcess()
        {
            while(true)
            {
                try
                {
                    string lstMessage = strReader.ReadLine();
                    if (lstMessage != null && lstMessage != "")
                    {
                        form1.SetText(lstMessage + "\r\n");
                    }
                }
                catch(System.Exception)
                {
                    break;
                }
            }
        }
    }
}
