using System.Collections;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace chatServer
{
    public partial class Form1 : Form
    {
        delegate void SetTextDelegate(string text);

        private bool isLive = false;
        TcpListener chatServerListener = new TcpListener(IPAddress.Parse("127.0.0.1"), 12000);
        public ArrayList clientSocketArray = new ArrayList();

        public Form1()
        {
            InitializeComponent();
        }

        private void serverStartButton_Click(object sender, EventArgs e)
        {
            try
            {
                if (isLive == false)
                {
                    chatServerListener.Start();
                    Thread waitThread = new Thread(new ThreadStart(AcceptClient));
                    waitThread.Start();

                    changeServerState(true);
                }
                else
                {
                    serverStop();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("채팅오류:" + ex.ToString());
            }
        }

        private void serverStop()
        {
            chatServerListener.Stop();
            lock (clientSocketArray)
            {
                foreach (Socket clientSocket in clientSocketArray)
                {
                    clientSocket.Close();
                }
            }
            clientSocketArray.Clear();

            changeServerState(false);
        }

        private void changeServerState(bool isLive)
        {
            this.isLive = isLive;
            if (isLive)
            {
                serverStateLabel.Text = "서버 동작 중";
                serverStartButton.Text = "서버 종료";
            }
            else
            {
                serverStateLabel.Text = "서버 중지 됨";
                serverStartButton.Text = "서버 시작";
            }
        }

        private void AcceptClient()
        {
            Socket socketClient = null;
            while (true)
            {
                try
                {
                    socketClient = chatServerListener.AcceptSocket();

                    ClientHandler clientHandler = new ClientHandler();
                    clientHandler.Setup(this, socketClient, this.chatMessageTextBox);
                    Thread chatThread = new Thread(new ThreadStart(clientHandler.Process));
                    chatThread.Start();
                    clientSocketArray.Add(socketClient);
                }
                catch
                {
                    break;
                }
            }
        }

        public void SetText(string text)
        {
            text += "\r\n";
            if (this.chatMessageTextBox.InvokeRequired)
            {
                this.Invoke(new SetTextDelegate(SetText), new object[] { text });
            }
            else
            {
                this.chatMessageTextBox.AppendText(text);
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            Application.Exit();
            serverStop();
        }
    }

    public class ClientHandler
    {
        private TextBox chatMessageTextBox;
        private Socket socketClient;
        private NetworkStream networkStream;
        private StreamReader streamReader;
        private Form1 form1;

        public void Setup(Form1 form1, Socket socketClient, TextBox chatMessageTextBox)
        {
            this.form1 = form1;
            this.socketClient = socketClient;
            this.chatMessageTextBox = chatMessageTextBox;
            this.networkStream = new NetworkStream(socketClient);
            this.streamReader = new StreamReader(networkStream);
        }

        public void Process()
        {
            while(true)
            {
                try
                {
                    string message = streamReader.ReadLine();
                    if(message != null && message != "")
                    {
                        form1.SetText(message);
                        byte[] sendData = Encoding.Default.GetBytes(message);
                        NetworkStream networkStream = null;
                        lock (form1.clientSocketArray)
                        {
                            foreach(Socket clientSocket in form1.clientSocketArray)
                            {
                                networkStream = new NetworkStream(clientSocket);
                                networkStream.Write(sendData, 0, sendData.Length);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("채팅오류:" + ex.ToString());
                    lock(form1.clientSocketArray)
                    {
                        form1.clientSocketArray.Remove(socketClient);
                    }
                }
            }
        }
    }
}
