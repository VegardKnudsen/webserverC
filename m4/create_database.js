const sqlite3 = require('sqlite3');
const db = new sqlite3.Database('books.db');

db.serialize(() => {
    db.run("CREATE TABLE user (userID NUMBER, passwordhash TEXT, firstname TEXT, lastname TEXT)");
    db.run("CREATE TABLE sessioninfo (sessionID NUMBER, userID NUMBER)");
    db.run("CREATE TABLE author (authorID NUMBER, firstname TEXT, lastname TEXT, nationality TEXT)");
    db.run("CREATE TABLE book (bookID NUMBER, title TEXT, authorID NUMBER)");

    db.run("INSERT INTO author VALUES (1, 'Ernest', 'Hemmingway', 'USA')");
    db.run("INSERT INTO book VALUES (1, 'The Sun Also Rises', 1)");

    console.log('Successfully created tables, and inserted data into author and book tables...');
    
    db.each("SELECT * FROM author", (err, row) => {
        console.log(row.authorID + ": " + row.firstname + ": " + row.lastname + ": " + row.nationality);
    });
});

db.close();