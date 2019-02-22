const port = 3000;

const express = require('express');
const sqlite3 = require('sqlite3');
const js2xmlparser = require('js2xmlparser');

const db = new sqlite3.Database('books.db');
const app = express();


app.get('/', (req, res) => {
    res.send('Default route');
});

app.get('/authors/:authorid', (req, res) => {
    const authorLookup = req.params.authorid;

    db.all(
        'SELECT * FROM author WHERE authorID=$authorID',
        {
            $authorID: authorLookup
        },
        (err, rows) => {
            //console.log(rows);
            if(rows.length > 0){
                res.send(js2xmlparser.parse("author", rows));
            } 
            else {
                res.send("Couldn't find anything...")
            }
        }
    );
});

app.get('/authors', (req, res) => {
    //const authorsLookup = req.params.authors;

    db.all(
        'SELECT * FROM author',
        (err, rows) => {
            console.log(rows);
            if(rows.length > 0){
                //res.send(rows);
                res.send(js2xmlparser.parse("authors", rows));
            } 
            else {
                res.send("Couldn't find anything...")
            }
        }
    );
});

app.get('/books/:bookid', (req, res) => {
    const bookLookup = req.params.bookid;

    db.all(
        'SELECT * FROM book WHERE bookID=$bookID',
        {
            $bookID: bookLookup
        },
        (err, rows) => {
            //console.log(rows);
            if(rows.length > 0){
                res.send(js2xmlparser.parse("book", rows));
            } 
            else {
                res.send("Couldn't find anything...")
            }
        }
    );
});

app.get('/books', (req, res) => {
    db.all(
        'SELECT * FROM book',
        (err, rows) => {
            console.log(rows);
            if(rows.length > 0){
                //res.send(rows);
                res.send(js2xmlparser.parse("books", rows));
            } 
            else {
                res.send("Couldn't find anything...");
            }
        }
    );
});

app.post('/post', (req, res) => {
    res.send("Post request");
})

app.post('/add/authors/:id/:firstname/:lastname/:nationality', (req, res) => {
    const idParam = req.params.id;
    const firstnameParam = req.params.firstname;
    const lastnameParam = req.params.lastname;
    const nationalityParam = req.params.nationality;

    db.run(
        'INSERT INTO authors VALUES (authorID=$authorID, firstname=$firstname, lastname=$lastname, nationailty=$nationality)',
        {
            $authorID: idParam,
            $firstname: firstnameParam,
            $lastname: lastnameParam,
            $nationality: nationalityParam
        },
        (err) => {
            if (err){
                console.log("Something wrong happened!");
            }
            else {
                console.log("Author added!");
            }
        }
    );
});

app.listen(port, () => console.log('Listening on port ' + port));