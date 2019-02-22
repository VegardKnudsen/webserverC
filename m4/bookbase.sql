CREATE TABLE user (
  userID	SMALLINT(5),
  passwd	VARCHAR(20),
  firstname	VARCHAR(100),
  lastname	VARCHAR(100),

  PRIMARY KEY (userID)
);

CREATE TABLE author (

  authorID	SMALLINT(5),
  firstname	VARCHAR(100),
  lastname	VARCHAR(100),
  nationality	VARCHAR(100),

  PRIMARY KEY (authorID)
);

CREATE TABLE book (

  bookID	SMALLINT(5),
  booktitle	VARCHAR(250),
  authorID 	SMALLINT(5),

  PRIMARY KEY (bookID),
  FOREIGN KEY (authorID) REFERENCES author (authorID)
);

CREATE TABLE sessiontable (
  sessionID	SMALLINT(5),
  userID	SMALLINT(5),

  PRIMARY KEY (sessionID),
  FOREIGN KEY (userID) REFERENCES user (userID)
);

DROP TABLE author;
DROP TABLE user;
DROP TABLE book;
DROP TABLE sessiontable;
