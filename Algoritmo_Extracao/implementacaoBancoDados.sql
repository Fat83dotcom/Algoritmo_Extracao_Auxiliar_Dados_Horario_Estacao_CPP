-- Active: 1693166653609@@127.0.0.1@5432@dados_estacao

-- ********************** Implementações Obrigatórias - Modelo Físico **********************

select current_database();

create database dados_estacao;

-- ********************** Schema tabelas_horarias **********************

create schema tabelas_horarias;

-- SELECT pg_get_serial_sequence('dado_diario', 'codigo');

-- SELECT last_value - 1 AS last_primary_key
-- FROM public.dado_diario_codigo_seq;

select codigo from dado_diario order by codigo desc limit 1;

CREATE TABLE IF NOT EXISTS "schema"."tableName"
	(codigo serial not null PRIMARY KEY,
	data_hora timestamp not null UNIQUE,
	codigo_gerenciador bigint default {},
	umidade double precision null,
	pressao double precision null,
	temp_int double precision null,
	temp_ext double precision null,
	FOREIGN KEY (codigo_gerenciador)
	REFERENCES gerenciador_tabelas_horarias (codigo)
	ON DELETE CASCADE);

	CREATE OR REPLACE FUNCTION drop_tables_except_one() RETURNS void AS $$
		#variable_conflict use_column
		DECLARE
			table_name text;
		BEGIN
			FOR table_name IN (SELECT table_name FROM information_schema.tables 
							WHERE table_schema = 'tabelas_horarias' AND table_name NOT IN ('29-08-2023', '30-08-2023')) LOOP
				EXECUTE 'DROP TABLE IF EXISTS tabelas_horarias."' || table_name || '" CASCADE';
			END LOOP;
		END;
	$$ LANGUAGE plpgsql;

SELECT drop_tables_except_one();

-- ********************** Schema public **********************

CREATE TABLE
    dado_diario (
        codigo serial not null primary key,
        dia timestamp not null unique,
        media_umidade double precision not null,
        minimo_umidade double precision not null,
        maximo_umidade double precision not null,
        mediana_umidade double precision not null,
        moda_umidade double precision not null,
        media_pressao double precision not null,
        minimo_pressao double precision not null,
        maximo_pressao double precision not null,
        mediana_pressao double precision not null,
        moda_pressao double precision not null,
        media_temp_int double precision not null,
        minimo_temp_int double precision not null,
        maximo_temp_int double precision not null,
        mediana_temp_int double precision not null,
        moda_temp_int double precision not null,
        media_temp_ext double precision not null,
        minimo_temp_ext double precision not null,
        maximo_temp_ext double precision not null,
        mediana_temp_ext double precision not null,
        moda_temp_ext double precision not null
    );
	
CREATE TABLE gerenciador_tabelas_horarias(
	codigo SERIAL PRIMARY KEY,
	data_tabela DATE NOT NULL UNIQUE
);

CREATE OR REPLACE VIEW MEDIAS_DIARIAS 
	as
	select
	    dia,
	    media_umidade,
	    media_pressao,
	    media_temp_int,
	    media_temp_ext
	from dado_diario
; 

CREATE OR REPLACE VIEW MEDIAS_MIN_MAX_TEMP 
	as
	select
	    dia,
	    media_temp_int,
	    minimo_temp_int,
	    maximo_temp_int,
	    media_temp_ext,
	    minimo_temp_ext,
	    maximo_temp_ext
	from
	DADO_DIARIO; 

CREATE OR REPLACE VIEW MINIMAS_TOTAIS 
	as
	select
	    codigo,
	    dia,
	    minimo_umidade,
	    minimo_pressao,
	    minimo_temp_int,
	    minimo_temp_ext
	from dado_diario
	order by dia
DESC; 

CREATE OR REPLACE VIEW MAXIMAS_TOTAIS as
	select
	    codigo,
	    dia,
	    maximo_umidade,
	    maximo_pressao,
	    maximo_temp_int,
	    maximo_temp_ext
	from dado_diario
	order by dia
DESC; 

CREATE OR REPLACE VIEW MEDIANAS_TOTAIS 
	as
	select
	    codigo,
	    dia,
	    mediana_umidade,
	    mediana_pressao,
	    mediana_temp_int,
	    mediana_temp_ext
	from dado_diario
	order by dia
DESC; 

CREATE OR REPLACE VIEW MODAS_TOTAIS 
	as
	select
	    codigo,
	    dia,
	    moda_umidade,
	    moda_pressao,
	    moda_temp_int,
	    moda_temp_ext
	from dado_diario
	order by dia
DESC; 

CREATE OR REPLACE VIEW MEDIAS_TOTAIS 
	as
	select
	    codigo,
	    dia,
	    media_umidade,
	    media_pressao,
	    media_temp_int,
	    media_temp_ext
	from dado_diario
	order by dia
DESC; 

CREATE OR REPLACE VIEW label_datas AS
SELECT codigo, dia FROM dado_diario
ORDER BY dia DESC;

SELECT dia, maximo_umidade FROM dado_diario
WHERE maximo_umidade = (SELECT MAX(maximo_umidade) FROM dado_diario)
AND
EXTRACT(YEAR FROM(SELECT dia))='2023' 
ORDER BY dia DESC;

SELECT dia, minimo_umidade FROM dado_diario
WHERE minimo_umidade = (SELECT MAX(minimo_umidade) FROM dado_diario)
AND
EXTRACT(YEAR FROM(SELECT dia))='2023';

SELECT dia, maximo_temp_ext FROM dado_diario
WHERE maximo_temp_ext = (SELECT MAX(maximo_temp_ext) FROM dado_diario WHERE EXTRACT(YEAR FROM(SELECT dia))='2023')
AND
EXTRACT(YEAR FROM(SELECT dia))='2023' 
ORDER BY dia DESC;

SELECT dia, minimo_temp_ext FROM dado_diario
WHERE minimo_temp_ext = 
(SELECT MIN(minimo_temp_ext) FROM dado_diario WHERE EXTRACT(YEAR FROM(SELECT dia))='2023')
AND
EXTRACT(YEAR FROM(SELECT dia))='2023' 

ORDER BY dia DESC;SELECT dia, maximo_pressao FROM dado_diario
WHERE maximo_pressao = (SELECT MAX(maximo_pressao) FROM dado_diario)
AND
EXTRACT(YEAR FROM(SELECT dia))='2023' 
ORDER BY dia DESC;

SELECT dia, minimo_pressao FROM dado_diario
WHERE minimo_pressao = (SELECT MAX(minimo_pressao) FROM dado_diario)
AND
EXTRACT(YEAR FROM(SELECT dia))='2023' 
ORDER BY dia DESC;

SELECT AVG(media_temp_ext) FROM
dado_diario WHERE EXTRACT(YEAR FROM(SELECT dia))='2023';

SELECT AVG(media_umidade) FROM
dado_diario WHERE EXTRACT(YEAR FROM(SELECT dia))='2023';

SELECT AVG(media_pressao) FROM
dado_diario WHERE EXTRACT(YEAR FROM(SELECT dia))='2023';

SELECT DISTINCT EXTRACT(YEAR FROM dia) as ano FROM dado_diario;

SELECT 0 AS id, AVG(maximo_umidade) FROM
dado_diario WHERE EXTRACT(YEAR FROM(SELECT dia))='2023';

-- ********************** Testes Abaixo **********************

select * from maximas_totais;

-- drop view medias_totais;

select minimo_pressao
from medias_diarias
where
    dia between '2022-1-1' and '2022-12-31'
    -- and media_temp_int <= 28
order by dia;

select *
from medias_min_max_temp
where
    dia between '2023-1-1' and '2023-6-2'
    and minimo_temp_int <= 20
order by dia;

select min(minimo_temp_int) from medias_min_max_temp;

select * from dado_diario where maximo_temp_int >= 60;

select * from dado_diario where dia BETWEEN '2022-01-01' AND '2022-04-01';

-- truncate table dado_diario;

-- drop table dado_diario cascade;

show datastyle;

select * from dado_diario;

create table teste(
	codigo serial not null primary key,
	nome varchar null
);

select * from tabelas_horarias."01-04-2023";

drop schema dado_horario

select * from "tabelas_horarias"."25-07-2023" order by codigo desc;

-- delete from "tabelas_horarias"."25-07-2023" where temp_int > 35;

CREATE TABLE IF NOT EXISTS "tabelas_horarias"."15-07-2023"(
            codigo serial not null PRIMARY KEY,
            data_hora timestamp not null UNIQUE,
            umidade double precision null,
            pressao double precision null,
            temp_int double precision null,
            temp_ext double precision null
            );

select * from dado_diario where codigo='1212';

select codigo, data_hora, temp_ext from "tabelas_horarias"."25-07-2023" 
where temp_ext=(select max(temp_ext) from "tabelas_horarias"."25-07-2023");

select * from "tabelas_horarias"."16-10-2022" order by codigo desc;


select count(*) from "tabelas_horarias"."25-07-2023";

drop table gerenciador_tabelas_horarias cascade;

select * from gerenciador_tabelas_horarias;

select * from "tabelas_horarias"."27-07-2023"
where data_hora between '2023-07-27 23:00:00' and '2023-07-27 23:59:59'
union all
select * from "tabelas_horarias"."28-07-2023"
where data_hora between '2023-07-28 00:00:00' and '2023-07-28 23:59:59'
union all
select * from "tabelas_horarias"."29-07-2023"
where data_hora between '2023-07-29 00:00:00' and '2023-07-29 01:30:59'
order by data_hora asc


SELECT 
    *
FROM 
    "tabelas_horarias"."27-07-2023"
WHERE 
    (EXTRACT(MINUTE FROM data_hora)=0 or EXTRACT(MINUTE FROM data_hora)=30)
	and
	EXTRACT(Second FROM data_hora)=0
union
SELECT 
    *
FROM 
    "tabelas_horarias"."28-07-2023"
WHERE 
    (EXTRACT(MINUTE FROM data_hora)=0 or EXTRACT(MINUTE FROM data_hora)=30)
	and
	EXTRACT(Second FROM data_hora)=0
	union
SELECT 
    *
FROM 
    "tabelas_horarias"."29-07-2023"
WHERE 
    (EXTRACT(MINUTE FROM data_hora)=0 or EXTRACT(MINUTE FROM data_hora)=30)
	and
	EXTRACT(Second FROM data_hora)=0
order by data_hora asc
	
SELECT name, setting FROM pg_settings WHERE name LIKE 'autovacuum%';

SELECT * FROM "tabelas_horarias"."19-08-2023"
        ORDER BY codigo DESC LIMIT 1;
