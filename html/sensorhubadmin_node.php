<?php
#*******************************************************************
#
# Funktionen zur Nodepflege
#
#*******************************************************************
function list_node() {
  global $db;
  global $tabledetails;
  echo "<center><table $tabledetails>";
  echo "<tr><th>Node#</th><th>Nodename</th><th>Aufstellungsort</th><th>Batterie Nennspannung</th><th>Batterie Leerspannung</th><th>Batterie Istspannung</th><th>&nbsp;</th></tr>";
  $results = $db->query("SELECT Node, Nodename, Location,  Battery_UN,  Battery_UE,  Battery_Act from node");
  while ($row = $results->fetchArray()) {
    echo "<tr class=block2><td>",$row[0],"</td><td>",$row[1],"</td><td>",$row[2],"</td><td>",$row[3],"</td><td>",$row[4],
         "</td><td>",$row[5],"</td><td>",
         "<button class=myButton value=$row[0] onclick=getresult(this.value,'node','edit_node');>Editieren</button>",
         "<button class=myButton value=$row[0] onclick=getresult(this.value,'node','delete_node');>L&ouml;schen</button>",
         "</td></tr>\n";
  }
  echo "<tr><td colspan=8 align=right>&nbsp;",
       "<button class=myButton value=0 onclick=getresult(this.value,'node','new_node');>Neuen Node anlegen</button>",
       "&nbsp;</td></tr>",
       "</table></center>\n";
}

function edit_node($mynode) {
  global $db;
  global $tabledetails_edit;
  echo "<center><table $tabledetails_edit><tr><th colspan=2>Editieren von Node $mynode</th></tr>";
  if ($mynode==0) {
    $mynodename="";
    $mylocation="";
    $mybatteryun="";
    $mybatteryue="";
    echo "<tr class=block2><td>Nodenummer:</td><td>",
         "<input id=node class=input value=''></td></tr>",
         "<tr class=block2><td>Nodename:</td><td>";
  } else {
    $results = $db->query("SELECT Nodename, Location, Battery_UN, Battery_UE from node where node = '$mynode' ");
    $row = $results->fetchArray();
    $mynodename=$row[0];
    $mylocation=$row[1];
    $mybatteryun=$row[2];
    $mybatteryue=$row[3];
    echo "<tr class=block2><td>Nodename:</td><td>",
         "<input type=hidden id=node value=$mynode>";
  }
  echo "<input class=input id=nodename value='",$mynodename,"'>","</td></tr>",
       "<tr class=block2><td>Aufstellungsort:</td><td><input class=input id=location value='",$mylocation,"'></td></tr>",
       "<tr class=block2><td>Batterie Nennspannung:</td><td><input class=input id=batteryun value='",$mybatteryun,"'>","</td></tr>",
       "<tr class=block2><td>Batterie Leerspannung:</td><td><input class=input id=batteryue value='",$mybatteryue,"'>","</td></tr>",
       "<tr><td colspan=2><center>&nbsp;";
  if ($mynode==0) {
    echo "<button class=myButton onclick=savenewnode();>Werte eintragen</button>";
  } else {
    echo "<button class=myButton onclick=updatenode();>Werte eintragen</button>";
  }
  echo "<button class=myButton onclick=listnode();>Abbruch</button>",
       "&nbsp;</center></td></tr>";
  echo "</table>",
       "</center>";
}

function update_node($mynode) {
  global $db;
  $mynodename=$_GET["nodename"];
  $mylocation=$_GET["location"];
  $mybatteryun=$_GET["batteryun"];
  $mybatteryue=$_GET["batteryue"];
  $sql = "update node set nodename = '$mynodename', location = '$mylocation', battery_UN = $mybatteryun, battery_UE = $mybatteryue where node = '$mynode' ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}

function savenew_node($mynode) {
  global $db;
  $mynodename=$_GET["nodename"];
  $mylocation=$_GET["location"];
  $mybatteryun=$_GET["batteryun"];
  $mybatteryue=$_GET["batteryue"];
  $sql="insert into node (node, nodename, location, battery_UN, battery_UE ) values( '$mynode','$mynodename', '$mylocation', $mybatteryun, $mybatteryue) ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}

function delete_node($mynode) {
  global $db;
  $sql="select count(*) from sensor where node = '$mynode' ";
  $results = $db->query($sql);
  $row = $results->fetchArray();
  if ( $row[0] > 0 ) {
      mymessage("error","Fehler: Sensordatensätze vorhanden, dieser Node kann nicht gelöscht werden!");
  } else {
    $sql="delete from node where node = '$mynode' ";
    if ( $db->exec($sql) ) {
      mymessage("info","gespeichert");
    } else {
      mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
    }
  }
}

?>