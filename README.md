sensorhub
=========

Still in an early state ... but working

For documentation goto http://wilmsn.github.io/sensorhub/

Environment:
============
- Server: Raspberry PI
- Nodes: "Arduino" like build with minimal cost based on ATmega328P 

Aim of the project:
===================
Build up a network of sensors and store and process the data on a server.
With:
- minimal hardware cost
- only free software
- low energy cost
 
Quick start guide:
=================
1. Go to your development directory

   cd ~/entw

2.  Clone the RF24 Repo  

    git clone https://github.com/wilmsn/RF24.git RF24  

3.  Change to the RF24 folder and compile it    

    cd RF24  
    sudo make install

4. Clone the RF24Network Repo  

    git clone https://github.com/wilmsn/RF24Network.git RF24Network  

5. Change to the RF24Network folder and compile it  

    cd RF24Network  
    sudo make install

6. Clone the sensorhub Repro

   git clone https://github.com/wilmsn/sensorhub.git sensorhub
   
7. Change to the sensorhub folder and compile it  
   
   cd sensorhub
   make
   sudo mkdir -p /var/database
   sudo touch  /var/database/sensorhub.db
   sudo chmod 666  /var/database/sensorhub.db

8. Do a test run
 
   ./sensorhubd -v9  #Just stop it with ctrl-c

Now its up to you:
==================
Feel free to fork, use or modify it.
=======
