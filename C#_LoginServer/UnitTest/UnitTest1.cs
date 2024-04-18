using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using LoginServer;

namespace UnitTest
{
    [TestClass]
    public class LoginTest
    {
        [TestMethod] // Method_scenario_expectedReturnVaule
        public void canBeCanceledBy_UserIsAdmin_returnsTrue()
        {
            // Arrange
            var reservation = new ReservationTest();

            // Act
            var result = reservation.canBeCanceledBy(new User(true));
            Console.WriteLine("������ ������ ��� �׽�Ʈ");

            // Assert
            Assert.IsTrue(result);
        }

        [TestMethod]
        public void canBeCanceledBy_SameUser_returnsTrue()
        {
            // Arrange
            User reservedUser = new User(false);
            var reservation = new ReservationTest { reservedUser = reservedUser };

            // Act
            var result = reservation.canBeCanceledBy(reservedUser);
            Console.WriteLine("������ ������ ��� �׽�Ʈ");

            // Assert
            Assert.IsTrue(result);

        }

        [TestMethod]
        public void canBeCanceledBy_AnotherUser_returnsFalse()
        {
            // Arrange
            User reservedUser = new User(false);
            var reservation = new ReservationTest { reservedUser = reservedUser };

            // Act
            var result = reservation.canBeCanceledBy(new User(false));
            Console.WriteLine("�ٸ� ������ ����ϴ� ��� �׽�Ʈ");

            // Assert
            Assert.IsFalse(result);

        }
    }
}