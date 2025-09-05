-- Sample SQL Server schema for testing the QxOrm+QtOrm integration

-- Create test database tables
CREATE TABLE Users (
    id INT IDENTITY(1,1) PRIMARY KEY,
    username NVARCHAR(50) NOT NULL UNIQUE,
    email NVARCHAR(255) NOT NULL,
    created_at DATETIME2 DEFAULT GETDATE()
);

CREATE TABLE Posts (
    id INT IDENTITY(1,1) PRIMARY KEY,
    title NVARCHAR(255) NOT NULL,
    content NVARCHAR(MAX),
    user_id INT NOT NULL,
    created_at DATETIME2 DEFAULT GETDATE(),
    CONSTRAINT FK_Posts_Users FOREIGN KEY (user_id) REFERENCES Users(id)
);

CREATE TABLE Comments (
    id INT IDENTITY(1,1) PRIMARY KEY,
    content NVARCHAR(MAX) NOT NULL,
    post_id INT NOT NULL,
    user_id INT NOT NULL,
    created_at DATETIME2 DEFAULT GETDATE(),
    CONSTRAINT FK_Comments_Posts FOREIGN KEY (post_id) REFERENCES Posts(id),
    CONSTRAINT FK_Comments_Users FOREIGN KEY (user_id) REFERENCES Users(id)
);

-- Add some sample data
INSERT INTO Users (username, email) VALUES 
    ('john_doe', 'john@example.com'),
    ('jane_smith', 'jane@example.com');

INSERT INTO Posts (title, content, user_id) VALUES 
    ('First Post', 'This is my first blog post!', 1),
    ('Another Post', 'This is another interesting post.', 2);

INSERT INTO Comments (content, post_id, user_id) VALUES 
    ('Great post!', 1, 2),
    ('Thanks for sharing!', 1, 1),
    ('Very informative.', 2, 1);