CREATE PROCEDURE mmo_account_find
    @id		VARCHAR
AS
    SELECT * FROM mmo_account where id = @id;
GO