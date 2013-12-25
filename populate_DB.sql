insert into Node (Nodeadr, Battery_Sensoradr, Battery_UN, Battery_UE, Location) 
values (1, 19, 3.0, 2.0,'Hauswand vorne');
insert into sensor (Nodeadr, Sensoradr, Sensortype, Sensorinfo, Nominal_Heartbeat) 
values (1, 11, 1, 'Temeratursensor', 50);
insert into sensor (Nodeadr, Sensoradr, Sensortype, Sensorinfo, Nominal_Heartbeat) 
values (1, 12, 1, 'Luftdrucksensor', 50);
insert into sensor (Nodeadr, Sensoradr, Sensortype, Sensorinfo, Nominal_Heartbeat) 
values (1, 19, 2, 'Batteriesensor von Node 1', 24);

