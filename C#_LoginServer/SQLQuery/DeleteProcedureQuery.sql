CREATE PROCEDURE mmo_account_find
    @id		VARCHAR
AS
    SELECT @id FROM mmo_account;
GO