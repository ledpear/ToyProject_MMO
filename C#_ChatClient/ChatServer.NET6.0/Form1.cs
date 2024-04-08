using System.Collections;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ChatServer.NET6._0
{
    delegate void SetTextDelegate(string s);

    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        TcpListener chatServer = new TcpListener(IPAddress.Parse("127.0.0.1"), 12000);
        public static ArrayList clientSocketArray = new ArrayList();
        private Thread waitThread = null;

        private void btnStart_Click(object sender, EventArgs e)
        {
            try
            {
                if (lblMsg.Tag.ToString() == "Stop")
                {
                    serverStart();
                }
                else
                {
                    serverStop();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("서버 시작 오류 :" + ex.Message);
            }
        }

        private void serverStart()
        {
            chatServer.Start();
            waitThread = new Thread(new ThreadStart(AcceptClient));
            waitThread.Start();

            lblMsg.Text = "Server 시작 됨";
            lblMsg.Tag = "Start";
            btnStart.Text = "서버 종료";
        }

        private void serverStop()
        {
            chatServer.Stop();
            foreach (Socket soket in Form1.clientSocketArray)
            {
                soket.Close();
            }
            clientSocketArray.Clear();

            lblMsg.Text = "Server 중지 됨";
            lblMsg.Tag = "Stop";
            btnStart.Text = "서버 시작";
        }

        private void AcceptClient()
        {
            Socket socketClient = null;
            while (true)
            {
                try
                {
                    socketClient = chatServer.AcceptSocket();

                    ClientHandle clientHandler = new ClientHandle();
                    clientHandler.ClientHandler_Setup(this, socketClient, this.txtChatMsg);
                    Thread thd_ChatProcess = new Thread(new ThreadStart(clientHandler.Chat_Process));
                    thd_ChatProcess.Start();

                    SetText("Connect Socket : " + socketClient.RemoteEndPoint.ToString() + "\r\n");
                }
                catch (Exception ex)
                {
                    Form1.clientSocketArray.Remove(socketClient);
                    break;
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

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            Application.Exit();
            serverStop();
        }
    }

    public class ClientHandle
    {
        private TextBox txtChatMsg;
        private Socket socketClient;
        private NetworkStream netStream;
        private StreamReader strReader;
        private Form1 form1;

        public void ClientHandler_Setup(Form1 form1, Socket socketClient, TextBox txtChatMsg)
        {
            this.txtChatMsg = txtChatMsg;
            this.socketClient = socketClient;
            this.netStream = new NetworkStream(socketClient);
            Form1.clientSocketArray.Add(socketClient);
            this.strReader = new StreamReader(netStream);
            this.form1 = form1;
        }

        public void Chat_Process()
        {
            while(true)
            {
                try
                {
                    string lstMessage = strReader.ReadLine();
                    if(lstMessage != null && lstMessage != "")
                    {
                        form1.SetText(lstMessage + "\r\n");
                        byte[] bytSand_Data = Encoding.Default.GetBytes(lstMessage + "\r\n");
                        lock(Form1.clientSocketArray)
                        {
                            foreach(Socket soket in Form1.clientSocketArray)
                            {
                                NetworkStream stream = new NetworkStream(soket);
                                stream.Write(bytSand_Data, 0, bytSand_Data.Length);
                            }
                        }
                    }
                }
                catch(Exception ex)
                {
                    MessageBox.Show("채팅 오류:" + ex.ToString());
                    Form1.clientSocketArray.Remove(socketClient);
                    break;
                }
            }
        }
    }
}
