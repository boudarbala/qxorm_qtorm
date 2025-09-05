-- Create Tables

CREATE TABLE Post (
id INT NULL,
title NVARCHAR(MAX) NULL,
content NVARCHAR(MAX) NULL,
userId INT NULL,
created_at DATETIME2 NULL
);

CREATE TABLE [User] (
id INT NULL,
username NVARCHAR(MAX) NULL,
email NVARCHAR(MAX) NULL,
created_at DATETIME2 NULL
);
