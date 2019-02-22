const sqlite3 = require('sqlite3');
const db = new sqlite3.Database('books.db');

db.serialize(() => {
    db.run("CREATE TABLE user (userID NUMBER, passwordhash TEXT, firstname TEXT, lastname TEXT, PRIMARY KEY (userID))");
    db.run("CREATE TABLE sessioninfo (sessionID NUMBER, userID NUMBER, PRIMARY KEY (sessionID), FOREIGN KEY (userID) REFERENCES user (userID))");
    db.run("CREATE TABLE author (authorID NUMBER, firstname TEXT, lastname TEXT, nationality TEXT, PRIMARY KEY (authorID))");
    db.run("CREATE TABLE book (bookID NUMBER, booktitle TEXT, authorID NUMBER, PRIMARY KEY (bookID), FOREIGN KEY (authorID) REFERENCES author (authorID))");

    db.run("INSERT INTO author VALUES (1, 'Ernest', 'Hemmingway', 'USA')");
    db.run("INSERT INTO author VALUES (2, 'Jo', 'Nesbø', 'Norway')");
    db.run("INSERT INTO book VALUES (1, 'The Sun Also Rises', 1)");
    db.run("INSERT INTO book VALUES (2, 'Hodejegerne', 2)");

    console.log('Successfully created tables, and inserted data into author and book tables...');
    
    db.each("SELECT * FROM author", (err, row) => {
        console.log(row.authorID + ": " + row.firstname + ": " + row.lastname + ": " + row.nationality);
    });
});

db.close();