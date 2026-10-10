DECLARE
    score NUMBER := 85;
    attendance NUMBER := 90;
BEGIN
    IF score >= 50 THEN
        DBMS_OUTPUT.PUT_LINE('Passed');

        IF attendance >= 75 THEN
            DBMS_OUTPUT.PUT_LINE('Attendance eligible');
        ELSE
            DBMS_OUTPUT.PUT_LINE('Attendance shortage');
        END IF;
    ELSE
        DBMS_OUTPUT.PUT_LINE('Failed');
    END IF;
END;
