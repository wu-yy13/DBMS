use test
create table table1 (id int(10), name varchar(10), primary key (id))
create table table2 (id int(10), name varchar(10), primary key (id))
create table table3 (id int(10), name varchar(10), primary key (id))
insert into table1 values (3, 'wy'),(2, 'have fun')
insert into table2 values (3, 'wy'), (2, 'have fun')
insert into table3 values (3, 'wuyy'), (2, 'have fun')
select table1.id from table1 where table1.name = 'wy'
update table1 set id = 1 where name = 'wy'
show tables
insert into table1 values (12, 'aaa'), (21, 'wangyan')
use test
select table1.id, table2.id from table1, table2 where table1.name = table2.name
select table1.id from table1 where table1.name = 'wyyyy'
select * from table1
drop table table2
show tables
desc table1
