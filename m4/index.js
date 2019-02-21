const port = 3000;

const express = require('express');
const sqlite3 = require('sqlite3');

const db = new sqlite3.Database('books.db');
const app = express();




app.get('/', (req, res) => {
    res.send('Default route');
});

app.get('/:table', (req, res) => {
    const tableToDisplay = req.params.table;

    db.all(
        'SELECT * FROM table=$table',
        {
            $table: tableToDisplay
        },
        (err, rows) => {
            console.log(rows);
        }
    );
});

app.get('/author/:authorid', (req, res) => {
    const authorLookup = req.params.authorid;

    db.all(
        'SELECT * FROM author WHERE authorID=$autorID',
        {
            $autorID: authorLookup
        },
        (err, rows) => {
            console.log(rows);
        }
    );
});

app.listen(port, () => console.log('Listening on port ' + port));