
DECLARE
    total NUMBER := 10;
    label VARCHAR2(20) := 'Total: ';
BEGIN
    total := total + 5;
    DBMS_OUTPUT.PUT_LINE(label || total);
END;
