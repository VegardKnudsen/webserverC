#!/bin/bash

echo    'Content-Type: text/html; charset=UTF-8'
echo    
echo    '<html>'
echo    '<head><title>CGI: Add book</title></head>'
echo    '<body>'
echo    '<h1>CGI Post</h1>'
echo    '<h2>Add a new book</h2>'
#action=address | address is the url of the cgi script the content should be sent to
echo    '<form method=POST action=localhost/m3/cgi/addbook.cgi'\
        'Book ID:<br>'\
        '<input type="number" name="bookid" value="">'\
        '<br>Book title:<br>'\
        '<input type="text" name="booktitle" value="">'\
        '<br>Author ID:<br>'\
        '<input type="number" name="authorid" value="">'\
        '<br><br>'\
        '<input type="submit" value="Submit">'\
        '</form>'

touch querystring.text
"$QUERY_STRING" > querystring.text



