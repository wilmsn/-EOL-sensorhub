insert into sensor (sensor,sensorinfo,node,channel)
            values(1, 'Temperatursensor auf Node 01','01', 1);
insert into sensor (sensor,sensorinfo,node,channel)
            values(2, 'Luftdrucksensor auf Node 01','01', 2);
insert into sensor (sensor,sensorinfo,node,channel)
            values(3, 'Batterie auf Node 01','01', 101);
insert into sensor (sensor,sensorinfo,node,channel)
            values(49, 'Batterie auf Node 04','04', 101);
insert into job (jobno,seq,sensor,value) values ( 10,1,'01', 1);
insert into job (jobno,seq,sensor,value) values ( 10,2,'01', 2);
insert into job (jobno,seq,sensor,value) values ( 10,3,'01', 101);
insert into schedule (schedule,jobno,start,interval) values (1,10, -1, 15);
insert into job (jobno,seq,sensor,value) values ( 11,1,'04', 101);
insert into schedule (schedule,jobno,start,interval) values (2,11, -1, 15);
insert into node (Node, Battery_UN, Battery_UE, Location) values( '01', 2.4, 2.0, 'Hauswand vorne');
insert into node (Node, Battery_UN, Battery_UE, Location) values( '04', 3.0, 2.0, 'nur Test');
insert into jobdesc (jobno,jobdesc) 
            values (10, 'Node 01: Temperatur, Luftdruck und Baterie abfragen');
insert into jobdesc (jobno,jobdesc)
            values (11, 'Node 04: Baterie abfragen');

