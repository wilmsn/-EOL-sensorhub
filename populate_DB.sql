insert into sensor values(1, 'Temperatursensor auf Node 01','01', 1,0,' ');
insert into sensor values(2, 'Luftdrucksensor auf Node 01','01', 2,0,' ');
insert into sensor values(3, 'Batterie auf Node 01','01', 101,0,' ');
insert into sensor values(49, 'Batterie auf Node 04','04', 101,0,' ');
insert into job values ( 10,1,'01', 1, 0);
insert into job values ( 10,2,'01', 2, 0);
insert into job values ( 10,3,'01', 101, 0);
insert into schedule values (10, -1, 15);
insert into job values ( 11,1,'04', 101, 0);
insert into schedule values (11, -1, 15);
insert into node values( '01', 2.4, 2.0, 'Hauswand vorne');
insert into node values( '04', 3.0, 2.0, 'nur Test');

