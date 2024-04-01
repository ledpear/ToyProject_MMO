namespace chatClient
{
    public partial class Form1 : Form
    {
        private bool isBefore = true;

        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if(isBefore)
            {
                label1.Text = "Test After";
                isBefore = false;
            }
            else
            {
                label1.Text = "Test Before";
                isBefore = true;
            }
        }
    }
}
