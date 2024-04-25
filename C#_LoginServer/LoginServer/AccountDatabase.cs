using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Data.SqlClient;
using System.Data;

namespace LoginServer
{
    internal class AccountDatabase
    {
        //private static string accountDBSource   = "192.168.10.199";
        //private static string accountDBName = "DESKTOP-4HM04BS";

        private static string accountDBSource   = "localhost";
        private static string accountDBName     = "master";
        private static string accountDBID       = "sa";
        private static string accountDBPassword = "1q2w3e4r";

        private static string dbConnectString;
        private static SqlConnection sqlConnection;

        static AccountDatabase()
        {
            dbConnectString =   $"Data Source={accountDBSource};" +
                                $"Initial Catalog={accountDBName};" +
                                $"User ID={accountDBID};" +
                                $"Password={accountDBPassword};";

            sqlConnection = new SqlConnection(dbConnectString);
        }

        public bool connectDB(ref string message)
        {
            if (sqlConnection.State == System.Data.ConnectionState.Open)
                return true;

            try
            {
                sqlConnection.Open();
            }
            catch (Exception ex)
            {
                message = "DB connect fail " + ex.Message;
                return false;
            }

            return true;
        }

        public bool findID(string id, ref string resultMessage)
        {
            DataSet dataSet = new DataSet();
            bool isFind = false;

            try
            {
                using (SqlCommand sqlCommand = new SqlCommand())
                {
                    sqlCommand.CommandText = "mmo_account_find";
                    sqlCommand.CommandType = CommandType.StoredProcedure;
                    sqlCommand.Connection = sqlConnection;

                    sqlCommand.Parameters.AddWithValue("@id", id);
                    using (SqlDataAdapter sqlDataAdapter = new SqlDataAdapter())
                    {
                        sqlDataAdapter.SelectCommand = sqlCommand;
                        sqlDataAdapter.Fill(dataSet);
                    }

                    if (dataSet.Tables[0].Rows.Count == 1)
                        isFind = true;
                }
            }
            catch(Exception ex)
            {
                resultMessage = ex.Message;
            }
            

            return isFind;
        }
    }
}
