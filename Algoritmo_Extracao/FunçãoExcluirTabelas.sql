CREATE OR REPLACE FUNCTION drop_tables_except_one() RETURNS void AS $$
#variable_conflict use_column
DECLARE
    table_name text;
BEGIN
    FOR table_name IN (SELECT table_name FROM information_schema.tables 
					   WHERE table_schema = 'tabelas_horarias' AND table_name != NOT IN ('29-08-2023', '30-08-2023')) LOOP
        EXECUTE 'DROP TABLE IF EXISTS tabelas_horarias."' || table_name || '" CASCADE';
    END LOOP;
END;
$$ LANGUAGE plpgsql;

SELECT drop_tables_except_one();