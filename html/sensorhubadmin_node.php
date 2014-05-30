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
	echo "<tr><th>Node#</th><th>Nodename</th><th>Aufstellungsort</th><th>Batterie<br>Nennspannung</th>",
		"<th>Batterie<br>Leerspannung</th><th>Batterie<br>Istspannung</th>",
		"<th>Sleeptime<br>(sek)</th><th>Radio-<br>mode</th><th>Infos</th><th>&nbsp;</th></tr>";
	$results = $db->query("SELECT Node, Nodename, Location,  Battery_UN,  Battery_UE,  Battery_Act, sleeptime, radiomode, add_info ".
						" ,julianday(date('now')) - ifnull((select julianday(date(max(last_ts))) from sensor where node = a.node),1) ". # Zur Feststellung ob der Node aktiv ist !!!
						" from node a ".
						" order by substr(Node,length(node),1), substr(Node,length(node)-1,1), substr(Node,length(node)-2,1) ");
	while ($row = $results->fetchArray()) {
	if ($row[9] > 1) { $td_col = "bgcolor=#999999"; } else { $td_col = ""; }
	echo "<tr class=block2><td $td_col align=right>",$row[0],"</td><td $td_col>",$row[1],"</td><td $td_col>",$row[2],"</td><td $td_col>",$row[3],"</td><td $td_col>",$row[4],
         "</td><td $td_col>",$row[5],"</td><td $td_col>",$row[6],"</td><td $td_col>";
	if ( $row[7] > 0 ) echo "ON"; else echo "Sleep";
	echo "</td>";
	if ( strlen($row[8]) > 2 ) {
		echo "<td bgcolor=#999999 align=center><a href='#' title='$row[8]'><img src=/img/info_rt.png width=20px height=20px></a></td>";
	} else {
		echo "<td $td_col>&nbsp;</td>";
	}	
	echo "<td $td_col>",
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
	$mysleeptime="";
	$myradiomode_str="<option selected value=0>Radio sleeps</option><option value=1>Radio on</option>";
	$myaddinfo="";
    echo "<tr class=block2><td>Nodenummer:</td><td>",
         "<input id=node class=input value=''></td></tr>",
         "<tr class=block2><td>Nodename:</td><td>";
  } else {
    $results = $db->query("SELECT Nodename, Location, Battery_UN, Battery_UE, sleeptime, radiomode, add_info from node where node = '$mynode' ");
    $row = $results->fetchArray();
    $mynodename=$row[0];
    $mylocation=$row[1];
    $mybatteryun=$row[2];
    $mybatteryue=$row[3];
	$mysleeptime=$row[4];
	if ( $row[5] == 0 ) {
		$myradiomode_str="<option selected value=0>Radio sleeps</option><option value=1>Radio on</option>";
	} else {
		$myradiomode_str="<option value=0>Radio sleeps</option><option selected value=1>Radio on</option>";
	}	
	$myaddinfo=$row[6];
    echo "<tr class=block2><td>Nodename:</td><td>",
         "<input type=hidden id=node value=$mynode>";
  }
  echo "<input class=input id=nodename value='",$mynodename,"'>","</td></tr>",
       "<tr class=block2><td>Aufstellungsort:</td><td><input class=input id=location value='",$mylocation,"'></td></tr>",
       "<tr class=block2><td>Batterie Nennspannung:</td><td><input class=input id=batteryun value='",$mybatteryun,"'>","</td></tr>",
       "<tr class=block2><td>Batterie Leerspannung:</td><td><input class=input id=batteryue value='",$mybatteryue,"'>","</td></tr>",
       "<tr class=block2><td>Sleeptime:</td><td><input class=input id=sleeptime value='",$mysleeptime,"'>","</td></tr>",
       "<tr class=block2><td>Radiomode:</td><td><select size=1 class=input id=radiomode>",$myradiomode_str,"</select></td></tr>",
       "<tr class=block2><td>Sonstige Infos:</td><td><textarea class=input id=addinfo>",$myaddinfo,"</textarea></td></tr>",	   
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
  $mysleeptime=$_GET["sleeptime"];
  $myradiomode=$_GET["radiomode"];
  $myaddinfo=$_GET["addinfo"];
  $sql = "update node set nodename = '$mynodename', location = '$mylocation', battery_UN = $mybatteryun, battery_UE = $mybatteryue, sleeptime = $mysleeptime, radiomode = $myradiomode, add_info = '$myaddinfo' where node = '$mynode' ";
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
  $mysleeptime=$_GET["sleeptime"];
  $myradiomode=$_GET["radiomode"];
  $myaddinfo=$_GET["addinfo"];
  $sql="insert into node (node, nodename, location, battery_UN, battery_UE, sleeptime, radiomode, add_info ) values( '$mynode','$mynodename', '$mylocation', $mybatteryun, $mybatteryue, $mysleeptime, $myradiomode, '$myaddinfo') ";
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