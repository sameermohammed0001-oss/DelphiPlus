BEGIN
    CREATE TABLE IF NOT EXISTS students (
        id INTEGER PRIMARY KEY,
        name TEXT NOT NULL,
        marks INTEGER
    );

    INSERT INTO students (name, marks)
    VALUES ('Sameer', 90);

    SELECT * FROM students;

    UPDATE students
    SET marks = 95
    WHERE name = 'Sameer';

    SELECT * FROM students;

    DELETE FROM students
    WHERE name = 'Sameer';

    SELECT * FROM students;
END;
