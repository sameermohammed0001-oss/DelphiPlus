DECLARE
    counter NUMBER := 1;
BEGIN
    WHILE counter <= 5 LOOP
        DBMS_OUTPUT.PUT_LINE(counter);
        counter := counter + 1;
    END LOOP;
END;
