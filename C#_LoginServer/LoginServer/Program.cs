namespace LoginServer
{
    public class ReservationTest
    {
        static void Main(string[] args)
        {
            AccountDatabase accountDatabase = new AccountDatabase();

            string resultMessage = "";
            if (accountDatabase.connectDB(ref resultMessage))
                Console.WriteLine("DB connect Success");
            else
                Console.WriteLine("DB connect Fail\r\n" + resultMessage);

            string findID = "1";
            if(accountDatabase.findID(findID, ref resultMessage))
                Console.WriteLine($"find id[{findID}] success");
            else
            {
                if(resultMessage.Length == 0)
                    Console.WriteLine($"find id[{findID}] fail");
                else
                    Console.WriteLine("findID Fail\r\n" + resultMessage);
            }
        }

        public User reservedUser { get; set; }

        public bool canBeCanceledBy(User user)
        {
            if (user.isAdmin == true)
                return true;

            if (user == reservedUser)
                return true;

            return false;
        }
    }

    public class User
    {
        public bool isAdmin;

        public User(bool isAdmin)
        {
            this.isAdmin = isAdmin;
        }
    }
}
