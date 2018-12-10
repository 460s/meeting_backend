DROP TABLE IF EXISTS meeting;

CREATE TABLE meeting (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT UNIQUE NOT NULL,
  description TEXT NOT NULL,
  address TEXT NOT NULL,
  published INTEGER NOT NULL,
  signup_description TEXT NOT NULL,
  signup_from_date INTEGER NOT NULL,
  signup_to_date INTEGER NOT NULL,
  from_date INTEGER NOT NULL,
  to_date INTEGER NOT NULL
);