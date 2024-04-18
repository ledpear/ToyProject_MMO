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
            Console.WriteLine("유저가 주인인 경우 테스트");

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
            Console.WriteLine("유저가 본인인 경우 테스트");

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
            Console.WriteLine("다른 유저가 취소하는 경우 테스트");

            // Assert
            Assert.IsFalse(result);

        }
    }
}