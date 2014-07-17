<?php
if (isset($_GET["action"]))  {$action=$_GET["action"];} else { $action=""; }
if (isset($_GET["display"])) {$display=$_GET["display"];} else { $display=""; }
if (isset($_GET["dsid"]))    {$dsid=$_GET["dsid"];} else { $dsid=0; }
# $window_innerwidth=$_GET["window_innerwidth"];
$DB_FILENAME="/var/database/sensorhub.db";
$db = new SQLite3($DB_FILENAME);
$basedir="/admin";
$url=$basedir."/sensorhubadmin.php";
# echo $_SERVER['HTTP_USER_AGENT'] . "\n\n";
#*************************************************************
#
# Allgemeine Funktionen
#
#*************************************************************

function mk_link($label,$parameter) {
    global $url;
	if ( $parameter != "" ) {
		$returnstr = "<a href='#' onclick=\"getcontent('".$url."?".$parameter."');\">$label</a>";
	} else {  
		$returnstr = "<a href='#' onclick=\"getcontent('".$url."');\">$label</a>";
	}
	return $returnstr;
}

function mk_button($myfunction,$label,$parameter,$class) {
	global $url;
	$onclickstr = "onclick=".$myfunction."('".$url;
	if ( $parameter != "" ) {
		$onclickstr = $onclickstr."?".$parameter;
	} else {  
		$onclickstr = $onclickstr."');";
	}
	$returnstr = "<button class=$class $onclickstr >$label</button>";
#	$returnstr = "<a href='#' $onclickstr >$label</a>";
	return $returnstr;
}

function mymessage($msgtyp,$msgtext) {
  echo "<center><table><tr><td>";
  switch ($msgtyp) {
    case "info":
      if ( $msgtext == "gespeichert" ) { $msgtext = " &Auml;nderungen gespeichert"; }
      echo "<img src=/img/info.png height=25 width=25></td><td>";
    break;
    case "error":
      echo "<img src=/img/stop.png height=25 width=25></td><td>";
    break;
  }
  echo "$msgtext </td></tr></table></center>\n";
}

function input_select_node($mynode) {
  global $db;
  echo "<select name='nodeid' id='nodeid' size=1 class='input'>";
  if ($mynode == "") { echo "<option value='' selected>---</option>"; }
  $results = $db->query("SELECT Node_id, Node_name from node");
  while ($row = $results->fetchArray()) {
  switch ($row[0]) {
    case $mynode:
      echo "<option value='",$row[0],"' selected>",$row[1],"</option>";
    break;
    default:
      echo "<option value='",$row[0],"' >",$row[1],"</option>";
    }
  }
  echo "</select>";
}

#*************************************************************
#
# Node Funktionen
#
#*************************************************************

function node_list() {
	global $db;
	echo "<center><table>";
	echo "<tr><th rowspan=2>Node#</th><th rowspan=2>Nodename</th><th rowspan=2>Batterie<br>Typ</th>",
		 "<th rowspan=2>Batterie<br>Spannung</th><th colspan=4>Sleeptime (sek)</th><th rowspan=2>Radio-<br>mode</th><th rowspan=2>Infos</th><th>&nbsp;</th></tr>",
		 "<tr><th>1</th><th>2</th><th>3</th><th>4</th><th>&nbsp;</th></tr>";
	$results = $db->query("SELECT Node_ID, Node_name, Battery_name, U_Batt, sleeptime1, sleeptime2, sleeptime3, sleeptime4, radiomode, add_info ".
						  " ,julianday(date('now')) - ifnull((select julianday(date(max(utime),'unixepoch')) from sensor where node_ID = a.node_ID),1) ". # Zur Feststellung ob der Node aktiv ist !!!
						  " from node a, battery b where a.battery_id = b.battery_id ".
						  " order by substr(Node_id,length(node_id),1) asc, substr(Node_id,length(node_id)-1,1) asc, substr(Node_id,length(node_id)-2,1) asc");
	while ($row = $results->fetchArray()) {
		if ($row[10] > 1) { $td_col = "bgcolor=#999999"; } else { $td_col = ""; }
		echo "<tr class=block2><td $td_col align=right>",$row[0],"</td><td $td_col>",$row[1],"</td><td $td_col>",$row[2],"</td><td $td_col>",$row[3],"</td><td $td_col>",$row[4],
			 "</td><td $td_col>",$row[5],"</td><td $td_col>",$row[6],"</td><td $td_col>",$row[7],"</td><td $td_col>";
		if ( $row[8] > 0 ) echo "ON"; else echo "Sleep";
		echo "</td>";
		if ( strlen($row[9]) > 2 ) {
			echo "<td bgcolor=#aaaaaa align=center><a href='#' title='$row[9]'><img src=/img/info_rt.png width=20px height=20px></a></td>";
		} else {
			echo "<td $td_col>&nbsp;</td>";
		}	
		echo "<td $td_col>",
			 "<button class=myButton onclick=getresult('node','node_edit','$row[0]');>Editieren</button>",
			 "<button class=myButton onclick=getresult('node','node_delete','$row[0]');>Löschen</button>",
			 "</td></tr>\n";
	}
	echo "<tr><td colspan=12 align=right>&nbsp;",
		 "<button class=myButton onclick=getresult('node','node_edit','0');>Neuen Node anlegen</button>", 
		 "&nbsp;</td></tr></table></center>\n";
}

function node_edit($mynode) {
	global $db;
	echo "<script> function updatenode(myaction) { var mynode=document.getElementById('node').value; var mynodename=document.getElementById('nodename').value; var mysleeptime1=document.getElementById('sleeptime1').value; ",
		 "var mysleeptime2=document.getElementById('sleeptime2').value; var mysleeptime3=document.getElementById('sleeptime3').value; var mysleeptime4=document.getElementById('sleeptime4').value; ",
		 "var myradiomode=document.getElementById('radiomode').value; var myaddinfo=document.getElementById('addinfo').value; var mybatteryid=document.getElementById('batteryid').value;",
		 "$.get(myprogramm,{ display : 'node', action : myaction, dsid : mynode, nodename : mynodename, sleeptime1 : mysleeptime1, sleeptime2 : mysleeptime2, sleeptime3 : mysleeptime3, sleeptime4 : mysleeptime4, radiomode : myradiomode, addinfo : myaddinfo, batteryid : mybatteryid }, function(data) { ",
         "$('#result').hide(); $('#result').html(data); $('#result').show(); }); } </script>";
	echo "<center><table><tr><th colspan=2>Editieren von Node $mynode</th></tr>";
	if ($mynode==0) {
		$mynodename="";
		$mynodeaddinfo="";
		$mysleeptime1="";
		$mysleeptime2="";
		$mysleeptime3="";
		$mysleeptime4="";
		$myradiomode_str="<option selected value=0>Radio sleeps</option><option value=1>Radio on</option>";
		$myaddinfo="";
		$mybatteryid=0;
		echo "<tr class=block2><td>Nodenummer:</td><td>",
			 "<input id=node class=input value=''></td></tr>",
			 "<tr class=block2><td>Nodename:</td><td>";
	} else {
		$results = $db->query("SELECT Node_name, Add_Info, U_Batt, sleeptime1, sleeptime2, sleeptime3, sleeptime4, radiomode, battery_id from node where node_id = '$mynode' ");
		$row = $results->fetchArray();
		$mynodename=$row[0];
		$myaddinfo=$row[1];
		$mysleeptime1=$row[3];
		$mysleeptime2=$row[4];
		$mysleeptime3=$row[5];
		$mysleeptime4=$row[6];
		if ( $row[7] == 0 ) {
			$myradiomode_str="<option selected value=0>Radio sleeps</option><option value=1>Radio on</option>";
		} else {
			$myradiomode_str="<option value=0>Radio sleeps</option><option selected value=1>Radio on</option>";
		}	
		$mybatteryid=$row[8];
		echo "<tr class=block2><td>Nodename:</td><td>",
			 "<input type=hidden id=node value=$mynode>";
	}
	echo "<input class=input id=nodename value='",$mynodename,"'>","</td></tr>",
		 "<tr class=block2><td>Sleeptime 1:</td><td><input class=input id=sleeptime1 value='",$mysleeptime1,"'>","</td></tr>",
		 "<tr class=block2><td>Sleeptime 2:</td><td><input class=input id=sleeptime2 value='",$mysleeptime2,"'>","</td></tr>",
		 "<tr class=block2><td>Sleeptime 3:</td><td><input class=input id=sleeptime3 value='",$mysleeptime3,"'>","</td></tr>",
		 "<tr class=block2><td>Sleeptime 4:</td><td><input class=input id=sleeptime4 value='",$mysleeptime4,"'>","</td></tr>",
		 "<tr class=block2><td>Radiomode:</td><td><select size=1 class=input id=radiomode>",$myradiomode_str,"</select></td></tr>",
		 "<tr class=block2><td>Batterie:</td><td><select size=1 class=input id=batteryid>";
	$sql="select battery_id, Battery_sel_txt from battery where battery_id = $mybatteryid ";
	$results = $db->query($sql);
	while ($row = $results->fetchArray()) {
		echo "<option value=$row[0] selected>$row[1]</option>";
	}
	$sql="select battery_id, Battery_sel_txt from battery where battery_id != $mybatteryid ";
	$results = $db->query($sql);
	while ($row = $results->fetchArray()) {
		echo "<option value=$row[0]>$row[1]</option>";
	}
	echo "</select></td></tr>",
		 "<tr class=block2><td>Sonstige Infos:</td><td><textarea class=input id=addinfo>",$myaddinfo,"</textarea></td></tr>",
		 "<tr><td colspan=2><center>&nbsp;";
  if ($mynode==0) {
    echo "<button class=myButton onclick=updatenode('node_savenew');>Werte eintragen</button>";
  } else {
    echo "<button class=myButton onclick=updatenode('node_update');>Werte eintragen</button>";
  }
  echo "<button class=myButton onclick=updatenode('node_list');>Abbruch</button>",
       "&nbsp;</center></td></tr>";
  echo "</table>",
       "</center>";
}

function node_update($mycase, $mynode) {
	global $db;
	$mynodename=$_GET["nodename"];
	$mysleeptime1=$_GET["sleeptime1"];
	$mysleeptime2=$_GET["sleeptime2"];
	$mysleeptime3=$_GET["sleeptime3"];
	$mysleeptime4=$_GET["sleeptime4"];
	$myradiomode=$_GET["radiomode"];
	$myaddinfo=$_GET["addinfo"];
	$mybatteryid=$_GET["batteryid"];
	$my_error=0;
	switch($mycase) {
		case "update":
			$sql = "update node set node_name = '$mynodename', sleeptime1 = $mysleeptime1, sleeptime2 = $mysleeptime2, sleeptime3 = $mysleeptime3, sleeptime4 = $mysleeptime4, radiomode = $myradiomode, add_info = '$myaddinfo', battery_id = $mybatteryid where node_id = '$mynode' ";
		break;
		case "savenew":
			$sql="insert into node (node_id, node_name, sleeptime1, sleeptime2, sleeptime3, sleeptime4, radiomode, add_info, battery_id ) values( '$mynode','$mynodename', '$mysleeptime1', $mysleeptime2, $mysleeptime3, $mysleeptime4, $myradiomode, '$myaddinfo', $mybatteryid ) ";
		break;
		case "delete":
			$sql="select count(*) from sensor where node_id = '$mynode' ";
			$results = $db->query($sql);
			$row = $results->fetchArray();
			if ( $row[0] > 0 ) {
				mymessage("error","Fehler: Sensordatensätze vorhanden, dieser Node kann nicht gelöscht werden!");
				$my_error=1;
			} else {
				$sql="delete from node where node_id = '$mynode' ";
			}
		break;
	}
	if ($my_error == 0) {
		if ( $db->exec($sql) ) {
			mymessage("info","gespeichert");
		} else {
			mymessage("error","Fehler: SQL: $sql <br>".$db->lastErrorMsg());
		}
		if ( $mycase == "update" || $mycase == "savenew" ) {
			$db->exec("insert into jobbuffer(Job_ID,Seq,Node_ID,Channel,Type,Value,Priority, utime) values(99,1,'$mynode',111,2,$mysleeptime1,1,strftime('%s',datetime('now')))");
			$db->exec("insert into jobbuffer(Job_ID,Seq,Node_ID,Channel,Type,Value,Priority, utime) values(99,2,'$mynode',112,2,$mysleeptime2,1,strftime('%s',datetime('now')))");
			$db->exec("insert into jobbuffer(Job_ID,Seq,Node_ID,Channel,Type,Value,Priority, utime) values(99,3,'$mynode',113,2,$mysleeptime3,1,strftime('%s',datetime('now')))");
			$db->exec("insert into jobbuffer(Job_ID,Seq,Node_ID,Channel,Type,Value,Priority, utime) values(99,4,'$mynode',114,2,$mysleeptime4,1,strftime('%s',datetime('now')))");
			$db->exec("insert into jobbuffer(Job_ID,Seq,Node_ID,Channel,Type,Value,Priority, utime) values(99,5,'$mynode',115,2,$myradiomode,1,strftime('%s',datetime('now')))");
			$key = ftok("/var/www/index.html", 'S');
			if(!msg_queue_exists($key)) mymessage("error","Werte nicht zum Node übertragen (1)"); 
			if(($msqid = msg_get_queue($key)) === FALSE) mymessage("error","Werte nicht zum Node übertragen (2)");
			if(!msg_send($msqid, 12, "1\0", false)) mymessage("error","Werte nicht zum Node übertragen (3)");
		}
	}
}

#*************************************************************
#
# Sensor und Aktor Funktionen
# Achtung: Alle nicht sichtbaren Variablen verwenden den Begriff "sensor*"
#
#*************************************************************

function sensor_list($item) {
	global $db;
	if ( $item == "sensor" ) {
		$label1="Sensor";
		$label2="Neuen Sensor anlegen";
		$sql = "SELECT Sensor_id, Sensor_name, Node_id, Channel, Value, datetime(utime, 'unixepoch','localtime'), julianday(date('now')) - julianday(date(utime,'unixepoch')) from sensor order by substr(Node_id,length(node_id),1), substr(Node_id,length(node_id)-1,1), substr(Node_id,length(node_id)-2,1) ";
	} else {
		$sql = "SELECT Actor_id, Actor_name, Node_id, Channel, Value, datetime(utime, 'unixepoch','localtime'), julianday(date('now')) - julianday(date(utime,'unixepoch')) from actor order by substr(Node_id,length(node_id),1), substr(Node_id,length(node_id)-1,1), substr(Node_id,length(node_id)-2,1) ";
		$label1="Actor";
		$label2="Neuen Actor anlegen";
	}
	echo "<center><table>";
	echo "<tr><th>$label1</th><th>Node</th><th>Channel</th><th>Aktueller Wert</th><th>Zeitpunkt</th><th>&nbsp;</th></tr>";
	$results = $db->query($sql);
	while ($row = $results->fetchArray()) {
		if ($row[6] > 1) { $td_col = "bgcolor=#999999"; } else { $td_col = ""; }
		echo "<tr class=block2><td $td_col>",$row[1],"</td><td $td_col align=right>",$row[2],"</td><td $td_col>",$row[3],"</td><td $td_col>",$row[4],"</td><td $td_col>",$row[5],"</td><td $td_col>",
			 "<button class=myButton value=$row[0] onclick=getresult('".$item."','".$item."_edit',$row[0]);>Editieren</button>",
			 "<button class=myButton value=$row[0] onclick=getresult('".$item."','".$item."_delete',$row[0]);>L&ouml;schen</button>",
			 "</td></tr>\n";
	}
	echo "<tr><td colspan=8 align=right>&nbsp;",
		 "<button class=myButton value=0 onclick=getresult('".$item."','".$item."_edit','0');>$label2</button>",
		 "&nbsp;</td></tr>",
		 "</table></center>\n";
}

function sensor_edit($item, $mysensor) {
	global $db;
	$newsensor=FALSE;
	if ( $item == "sensor" ) {
		$sql1 = "SELECT ifnull(max(Sensor_id),0)+1 from sensor ";
		$label1 = "Anlegen eines neuen Sensors";
		$sql2 = "SELECT Sensor_id, Sensor_name, Node_id, Channel from sensor where sensor_id = $mysensor ";
		$label2 = "Editieren eines Sensors";
		$label3 = "Sensor anlegen";
		$label4 = "Sensorname:";
	} else {
		$sql1 = "SELECT ifnull(max(Actor_id),0)+1 from Actor ";
		$label1 = "Anlegen eines neuen Aktors";
		$sql2 = "SELECT Actor_id, Actor_name, Node_id, Channel from Actor where actor_id = $mysensor ";
		$label2 = "Editieren eines Aktors";
		$label3 = "Aktor anlegen";
		$label4 = "Aktorname:";
	}
	echo "<script> function updatesensor(myaction, myitem) { ",
		 "var mysensor=document.getElementById('sensorid').value; var mysensorname=document.getElementById('sensorname').value; ",
		 "var mynode=document.getElementById('nodeid').value; var mychannel=document.getElementById('channel').value; ",
		 "$.get(myprogramm,{ display : myitem, action : myaction, dsid : mysensor, sensorname : mysensorname, node : mynode, channel : mychannel}, function(data) { ",
		 "$('#result').hide(); $('#result').html(data); $('#result').show(); }); } </script>";
    echo "<center><table><tr><th colspan=2>";
	if ($mysensor==0) {
		$newsensor=TRUE;
		$results = $db->query($sql1);
		$row = $results->fetchArray();
		$mysensor=$row[0];
		$mysensorname="";
		$mynode="";
		$mychannel="";
		echo $label1." (".$mysensor.")";
	} else {
		$results = $db->query($sql2);
		$row = $results->fetchArray();
		$mysensorname=$row[1];
		$mynode=$row[2];
		$mychannel=$row[3];
		echo $label2." (".$mysensor.")";
	}
	echo "</th></tr>",
		 "<tr class=block2><td>$label4</td><td>",
         "<input type=hidden id=sensorid value=$mysensor>",
		 "<input class=input id=sensorname value='",$mysensorname,"'>","</td></tr>",
		 "<tr class=block2><td>Node:</td><td>";
	input_select_node($mynode);
	echo "</td></tr>",
		 "<tr class=block2><td>Channel:</td><td><input class=input id=channel value='",$mychannel,"'>","</td></tr>",
         "<tr><td colspan=2><center>&nbsp;";
	if ($newsensor) {
		echo "<button class=myButton onclick=updatesensor('".$item."_savenew','".$item."');>$label3</button>";
	} else {
		echo "<button class=myButton onclick=updatesensor('".$item."_update','".$item."');>Werte eintragen</button>";
	}
	echo "<button class=myButton onclick=updatesensor('".$item."_list','".$item."');>Abbruch</button>",
		 "&nbsp;</center></td></tr>",
	     "</table>",
		 "</center>";
}

function sensor_update($item, $mycase, $mysensor) {
	global $db;
	$mysensorname=$_GET["sensorname"];
	$mynode=$_GET["node"];
	$mychannel=$_GET["channel"];
	$my_error=0;
	if ( $item == "sensor" ) { 
		$label1 = "Fehler: Jobdatensätze vorhanden, dieser Sensor kann nicht gelöscht werden!";
		$mytype = "= 1"; 
	} else { 
		$label1 = "Fehler: Jobdatensätze vorhanden, dieser Aktor kann nicht gelöscht werden!";
		$mytype = "> 1"; 
	}
	switch($mycase) {
		case "update":
			$sql="update ".$item." set node_id = '$mynode', channel = $mychannel, ".$item."_name = '$mysensorname' where ".$item."_id = $mysensor ";
		break;
		case "savenew":
			$sql="insert into ".$item." (".$item."_id, ".$item."_name, node_id, channel, utime) values( $mysensor,'$mysensorname', '$mynode', $mychannel, 0) ";
		break;
		case "delete":
			$sql="select count(*) from jobstep where id = $mysensor and type ".$mytype;
			echo $sql;
			$results = $db->query($sql);
			$row = $results->fetchArray();
			if ( $row[0] > 0 ) {
				mymessage("error",$label1);
				$my_error=1;
			} else {
				$sql="delete from ".$item." where ".$item."_id = $mysensor ";
			}
		break;
	}
	if ( $my_error == 0 ) {
		if ( $db->exec($sql) ) {
			mymessage("info","gespeichert");
		} else {
			mymessage("error","Fehler: SQL: $sql <br>".$db->lastErrorMsg());
		}
	}
}

#*************************************************************
#
# Trigger Funktionen
#
#*************************************************************

function trigger_list() {
	global $db;
	echo "<style type='text/css'> td, th, tr, table { border:thin solid black; } </style>";
	$td_col="bgcolor=#999999";
	echo "<center><table>";
	echo "<tr><th>Trigger</th><th>Sensor</th><th>Trigger<br>Ausl&ouml;sewert</th><th>Trigger<br>R&uuml;cksetzwert</th><th>Trigger<br>Status</th><th>aktueller<br>Wert</th><th>Infos</th><th>&nbsp;</th></tr>";
	$results = $db->query("SELECT a.trigger_id, a.trigger_name, b.sensor_name, a.level_set, a.level_reset, a.state, a.Add_Info, b.value ".
						  "from trigger a, sensor b ".
						  "where a.sensor_id = b.sensor_id order by trigger_id  ");
	while ($row = $results->fetchArray()) {
#   if ($row[6] > 1) { $td_col = "bgcolor=#999999"; } else { $td_col = ""; }
		echo "<tr class=block2><td>",$row[1],"</td><td align=right>",$row[2],"</td><td>",$row[3],"</td><td>",
			  $row[4],"</td><td>";
		if ($row[5] == 's') echo "ausgel&oumlst"; else echo "zur&uuml;ckgesetzt";
		echo "</td><td>$row[7]</td>";
		if ( strlen($row[6]) > 2 ) {
			echo "<td bgcolor='#aaaaaa' align=center><a href='#' title='$row[6]'><img src=/img/info_rt.png width=20px height=20px></a></td>";
		} else {
			echo "<td>&nbsp;</td>";
		}	
		echo "</td><td>",
			 "<button class=myButton onclick=getresult('trigger','trigger_edit',$row[0]);>Editieren</button>",
			 "<button class=myButton onclick=getresult('trigger','trigger_delete',$row[0]);>L&ouml;schen</button>",
			 "</td></tr>\n";
	}
	echo "<tr><td colspan=8 align=right>&nbsp;",
		 "<button class=myButton onclick=getresult('trigger','trigger_edit','0');>Neuen Trigger anlegen</button>",
		 "&nbsp;</td></tr>",
		 "</table></center>\n";
}

function trigger_edit($mytrigger) {
	global $db;
	$newtrigger=0;
	echo "<script> function updatetrigger(myaction) { ",
		 "var mytrigger=document.getElementById('triggerid').value; var mytriggername=document.getElementById('triggername').value; ",
		 "var mysetlevel=document.getElementById('setlevel').value; var myresetlevel=document.getElementById('resetlevel').value; ",
		 "var mystate=document.getElementById('state').value; var myaddinfo=document.getElementById('addinfo').value; ",
		 "var mysensorid=document.getElementById('sensorid').value; ",
		 "$.get(myprogramm,{ display : 'trigger', action : myaction, dsid : mytrigger, triggername : mytriggername, setlevel : mysetlevel, resetlevel : myresetlevel, state : mystate, addinfo : myaddinfo, sensorid : mysensorid}, function(data) { ",
		 "$('#result').hide(); $('#result').html(data); $('#result').show(); }); } </script>";
	echo "<center><table><tr><th colspan=2>";
	if ($mytrigger==0) {
		$results = $db->query("SELECT ifnull(max(Trigger_ID),0)+1 from Trigger");
		$row = $results->fetchArray();
		$mytrigger=$row[0];
		$mytriggername="";
		$myaddinfo=""; 
		$mysetlevel=0;
		$myresetlevel=0;
		$mysensorid=0;
		$mystate="r";
		$newtrigger=1;
		echo "Anlegen eines neuen Triggers (".$mytrigger.")";
	} else {
		$results = $db->query("SELECT trigger_id, Trigger_name, Add_Info, Level_Set, Level_Reset, State, Sensor_ID from Trigger where Trigger_ID = $mytrigger ");
		$row = $results->fetchArray();
		$mytrigger=$row[0];
		$mytriggername=$row[1];
		$myaddinfo=$row[2];
		$mysetlevel=$row[3];
		$myresetlevel=$row[4];
		$mystate=$row[5];
		$mysensorid=$row[6];
		echo "Editieren eines Triggers (".$mytrigger.")";     
	}
	echo "</th></tr>",
		 "<tr class=block2><td>Triggername:</td><td>",
		 "<input type=hidden id=triggerid value=$row[0]>",
		 "<input class=input id=triggername value='",$mytriggername,"'>","</td></tr>",
         "<tr class=block2><td>Setzwert:</td><td><input class=input id=setlevel value='",$mysetlevel,"'>","</td></tr>",
		 "<tr class=block2><td>Rücksetzwert:</td><td><input class=input id=resetlevel value='",$myresetlevel,"'>","</td></tr>",
		 "<tr class=block2><td>Status:</td><td><select size=1 class=input id=state>";
	if ( $mystate == "r" ) {
		echo "<option value='r' selected>zurückgesetzt</option><option value='s'>gesetzt</option>";
	} else {
		echo "<option value='s' selected>gesetzt</option><option value='r'>zurückgesetzt</option>";
	}
	echo "</select></td></tr>",
	     "<tr class=block2><td>Sensor:</td><td><select size=1 id=sensorid class=input>";
	if ( $mysensorid == 0 ) echo "<option 0 selected> --- </option>";
	$results = $db->query("SELECT sensor_id, sensor_name from sensor ");
	while (	$row = $results->fetchArray() ) {
		if ( $row[0] == $mysensorid ) { 
			echo "<option value=$row[0] selected>$row[1]</option>";
		} else {
			echo "<option value=$row[0]>$row[1]</option>";
		} 		
	}
	echo "</select></td></tr>",
		 "<tr class=block2><td>Sonstige Infos:</td><td><textarea class=input id=addinfo>",$myaddinfo,"</textarea></td></tr>",
		 "<tr><td colspan=2><center>&nbsp;";
  if ($newtrigger==1) {
    echo "<button class=myButton onclick=updatetrigger('trigger_savenew');>Trigger anlegen</button>";
  } else {
    echo "<button class=myButton onclick=updatetrigger('trigger_update');>Werte eintragen</button>";
  }
  echo "<button class=myButton onclick=updatetrigger('trigger_list');>Abbruch</button>",
       "&nbsp;</center></td></tr></table></center>";
}

function trigger_update($mycase, $mytrigger) {
	global $db;
	$my_error = 0;
	if ( ! ($mycase == "delete") ) {
		$mytriggername=$_GET["triggername"];
		$myaddinfo=$_GET["addinfo"];
		$mysetlevel=$_GET["setlevel"];
		$myresetlevel=$_GET["resetlevel"];
		$mystate=$_GET["state"];
		$mysensorid=$_GET["sensorid"];
		if ( $mysetlevel == $myresetlevel ) {
			$my_error = 1;
			mymessage("error","Fehler: Setzwert und Rücksetzwert müssen sich beim Trigger unterscheiden!");
		}
		if ( $mysensorid == 0 ) {
			$my_error = 1;
			mymessage("error","Fehler: Bitte einen Sensor als Basis für diesen Trigger auswählen!");
		}
		if ( strlen($mytrigger) == 0 ) {
			$my_error = 1;
			mymessage("error","Fehler: Bitte einen Namen für diesen Trigger vergeben!");
		}
	}
	switch($mycase) {
		case "update":
			$sql = "update trigger set trigger_name = '$mytriggername', level_set = $mysetlevel, level_reset = $myresetlevel, state = '$mystate', sensor_id = $mysensorid, add_info = '$myaddinfo' where trigger_id = '$mytrigger' ";
		break;
		case "savenew":
			$sql="insert into trigger (trigger_id, trigger_name, level_set, level_reset, state, add_info, sensor_id ) values( $mytrigger,'$mytriggername', $mysetlevel, $myresetlevel, '$mystate', '$myaddinfo', $mysensorid ) ";
		break;
		case "delete":
			$sql="select count(*)  from schedule where Triggered_By = 'v' and trigger_id =  $mytrigger  ";
			$results = $db->query($sql);
			$row = $results->fetchArray();
			if ( $row[0] > 0 ) {
				mymessage("error","Fehler: Jobdatensätze vorhanden, dieser Trigger kann nicht gelöscht werden!");
				$my_error=1;
			} else {
				$sql="delete from trigger where trigger_id = '$mytrigger' ";
			}
		break;
	}
	if ($my_error == 0) {
		if ( $db->exec($sql) ) {
			mymessage("info","gespeichert");
		} else {
			mymessage("error","Fehler: SQL: $sql <br>".$db->lastErrorMsg());
		}
	}
}

#*************************************************************
#
# Job Funktionen
#
#*************************************************************

function job_list() {
	global $db;
	if (isset($_GET["seq"]))    {$myseq=$_GET["seq"];} else { $myseq=0; }
	echo "<center><table>";
	echo "<tr class=block1><th>Jobkette</th><th>Aktion</th><th>&nbsp;</th></tr>";
	$results = $db->query("SELECT job_id, Job_name, add_info from Job");
	while ($row = $results->fetchArray()) {
		$myjob=$row[0];
		$myjobname=$row[1];
		$myaddinfo=$row[2];
		$results1 = $db->query("SELECT count(*)+1 from Jobstep where job_id = $myjob ");
		$row1 = $results1->fetchArray();
		$row_count=$row1[0];
		echo "<tr class=block2><td rowspan=$row_count>$myjobname<br>$myaddinfo<br><button class=myButton onclick=getresult('job','job_edit','".$myjob."');>Editieren</button></td>";
		$results1 = $db->query("SELECT seq, add_info from jobstep where job_ID = $myjob order by Seq asc");
#        if ($results1->rowCount() == 0 ) echo "<td>&nbsp;</td><td>&nbsp;</td></tr>",
		$i=0;
		while ($row1 = $results1->fetchArray()) {
		    if ( $i > 0 ) echo "<tr class=block2>";
			echo "<td>$row1[1]</td><td>",
				 "<button class=myButton onclick=getresult('job','job_edit','".$myjob.".".$row1[0]."');>Editieren</button>",
				 "<button class=myButton onclick=getresult('job','job_delete','".$myjob.".".$row1[0]."');>L&ouml;schen</button>",
				 "</td></tr>\n";
				 $i++;
		}
		if ($i == 0 ) echo "<td>xxxx</td><td>xxx</td></tr>";
		echo "<tr><td><button class=myButton value=$row[0] onclick=getresult('job','job_edit','".$myjob.".0');>Neuen Job in dieser Jobkette anlegen</button>";
		if ($i == 0 ) echo "<button class=myButton value=$row[0] onclick=getresult('job','job_delete','".$myjob.".');>Diesen Job löschen</button>";
		echo "</td></tr>",
			 "<tr><td colspan=4>&nbsp;",
			 "<button class=myButton value=$row[0] onclick=getresult('job','job_start_once',$myjob);>Jobkette sofort und einmalig starten</button>",
			 "&nbsp;",
			 "<button class=myButton value=$row[0] onclick=getresult('job','job_start_interval',$myjob);>Jobkette sofort mit Interval 10 min starten</button>",
			 "&nbsp;</td></tr>";
	}
	echo "</tr>",
		 "<tr><td colspan=8 align=right>&nbsp;",
		 "<button class=myButton onclick=getresult('job','job_edit','0.');>Neue Jobkette anlegen</button>",
		 "&nbsp;</td></tr>",
		 "</table></center>\n";
}

function job_edit($myjob) {
	global $db;
	$isnewjob=FALSE;
	$isnewjobstep=FALSE;
	$iseditjob=FALSE;
	$iseditjobstep=FALSE;
	$myjob_part=explode(".",$myjob);
	if ( $myjob_part[1] == "" ) {
		if ( $myjob_part[0] == 0 ) {
			$label1 = "Anlegen einer neuen Jobkette";
			$results = $db->query(" select ifnull(max(job_id)+1,1001) from job where job_id > 1000 ");
			$row = $results->fetchArray();
			$myjobname="";
			$myaddinfo="";
			$action="job_savenew";
			$isnewjob=TRUE;
		} else {
			$label1 = "Editieren einer Jobkette";
			$results = $db->query(" select job_name, add_info from job where job_id = $myjob_part[0] ");
			$row = $results->fetchArray();
			$myjobname=$row[0];
			$myaddinfo=$row[1];
			$action="job_update";
			$iseditjob=TRUE;
		}
	} else {	
		$results = $db->query(" select job_name, add_info from job where job_id = $myjob_part[0] ");
		$row = $results->fetchArray();
		$myjobname=$row[0];
		$myaddinfo=$row[1];
		if ( $myjob_part[1] == 0 ) {
			$label1 = "Anlegen einer neuen Aktion";
			$results = $db->query(" select ifnull(max(seq)+1,1) from jobstep where job_id = ".$myjob_part[0]);
			$row = $results->fetchArray();
			$myjob_part[1]=$row[0];
			$action="job_savenew";
			$isnewjobstep=TRUE;
			$myaddinfo="";
			$mytype=0;
			$myid=0;
			$myvalue=0;
			$mysensorid=0;
		} else {
			$label1 = "Bearbeiten einer Aktion";
			$action="job_update";
			$iseditjobstep=TRUE;
			$results = $db->query(" select add_info, type, ID, Value, sensor_id from jobstep where job_id = ".$myjob_part[0]." and seq = ".$myjob_part[1] );
			$row = $results->fetchArray();
			$myaddinfo=$row[0];
			$mytype=$row[1];
			$myid=$row[2];
			$myvalue=$row[3];
			$mysensorid=$row[4];			
		}
		$html_sensor="<select  size=1 class=input id=id>";
		$html_sensorid="<select  size=1 class=input id=sensorid>";
		$isselected="";
		$isselected_s="";
		if ( $mysensorid == 0 ) $html_sensorid=$html_sensorid."<option value=0 selected>---</option>";
		if ( $mytype > 1 || ($myid == 0 && mytype == 1) ) $html_sensor=$html_sensor."<option value=0 selected>---</option>";
		$results = $db->query(" select sensor_id, sensor_name from sensor " );
		while ($row = $results->fetchArray()) {
			if ( $mysensorid == $row[0] ) {$isselected_s="selected";} else {$isselected_s="";}
			$html_sensorid=$html_sensorid."<option value=$row[0] ".$isselected_s.">$row[1]</option>";
			if ($mytype == 1 and $myid == $row[0] ) { $isselected="selected";} else {$isselected="";}
			$html_sensor=$html_sensor."<option value=$row[0] ".$isselected.">$row[1]</option>";
		}
		$html_sensor=$html_sensor."</select>";
		$html_sensorid=$html_sensorid."</select>";
		$html_actor="<select  size=1 class=input id=id>";
		if ( $mytype == 1 || ($myid == 0 && mytype > 1 )) $html_actor=$html_actor."<option value=0 selected>---</option>";
		$results = $db->query(" select actor_id, actor_name from actor " );
		while ($row = $results->fetchArray()) {
			if ( $myid == $row[0] && $mytype > 1 ) { $isselected="selected";} else {$isselected="";}
			$html_actor=$html_actor."<option value=$row[0] $isselected>$row[1]</option>";
		}
		$html_actor=$html_actor."</select>";
	}
	echo "<center><input type=hidden id=job value='".$myjob_part[0].".".$myjob_part[1]."'><table><tr><th colspan=2>$label1</th></tr>";
    if ( $isnewjob || $iseditjob ) {
		echo "<script> function updatejob(myaction) { ",
			 "var myjob=document.getElementById('job').value; var myjobname=document.getElementById('jobname').value; var myaddinfo=document.getElementById('addinfo').value; ",
			 "$.get(myprogramm,{ display : 'job', action : myaction, dsid : myjob, jobname : myjobname, addinfo : myaddinfo }, function(data) { ",
			 "$('#result').hide(); $('#result').html(data); $('#result').show(); }); } </script>";
		echo "<tr><td>Name der Jobkette</td><td><input class=input size=50 maxlength=50 id=jobname value='".$myjobname."'></td></tr>",
			 "<tr><td>Zusätzliche Infos</td><td><textarea class=input id=addinfo>".$myaddinfo."</textarea></td></tr>";
	}
	if ( $iseditjobstep || $isnewjobstep ) {
		echo "<script> function updatejob(myaction) { ",
			 "var myjob=document.getElementById('job').value; var mytype=document.getElementById('type').value; var myaddinfo=document.getElementById('addinfo').value; ",
			 "var myid=document.getElementById('id').value; var myvalue=document.getElementById('value').value; var mysensorid=document.getElementById('sensorid').value; ",
			 "$.get(myprogramm,{ display : 'job', action : myaction, dsid : myjob, addinfo : myaddinfo, type : mytype, id : myid, value : myvalue, sensorid : mysensorid }, function(data) { ",
			 "$('#result').hide(); $('#result').html(data); $('#result').show(); }); } ",
			 " function hide_type_tr(value) { ",
			 "  \$('#job-tr1').hide(); ",
			 "   if ( value == '0' ) {",
			 "  \$('#job-tr1').hide(); ",
			 "  \$('#job-tr2').hide(); ",
			 "  \$('#job-tr3').hide(); ",
			 " } ",
			 "   if ( value == '1' ) {",
			 "  \$('#job-tr1').hide(); ",
			 "  \$('#job-tr2').hide(); ",
			 "  \$('#job-tr3').hide(); ",
			 "  \$('#job-tr1').html('<td>Sensor</td><td>".$html_sensor."</td>'); ",
			 "  \$('#job-tr1').show(); ",
			 " } ",
			 "   if ( value == '2' ) { ",
			 "  \$('#job-tr1').hide(); ",
			 "  \$('#job-tr2').show(); ",
			 "  \$('#job-tr3').hide(); ",
			 "  \$('#job-tr1').html('<td>Aktor</td><td>".$html_actor."</td>'); ",
			 "  \$('#job-tr1').show(); ",
			 " } ",	
			 "   if ( value == '3' ) { ",
			 "  \$('#job-tr1').hide(); ",
			 "  \$('#job-tr2').hide(); ",
			 "  \$('#job-tr3').show(); ",
			 "  \$('#job-tr1').html('<td>Aktor</td><td>".$html_actor."</td>'); ",
			 "  \$('#job-tr1').show(); ",
			 " } ",
			 "} ",
			 "hide_type_tr('".$mytype."'); ",
			 "</script>";
		echo "<tr><td>Name der Jobkette</td><td>".$myjobname."</td></tr>",
			 "<tr class=block2><td>Aktion</td><td><input class=input size=50 maxlength=50 id=addinfo value='".$myaddinfo."'></td></tr>",
			 "<tr class=block2><td>Typ</td><td><select size=1 class=input id=type onChange='hide_type_tr(this.value)'>";
		if ( $mytype == 0 ) echo "<option value=0 selected>---</option><option value=1>Sensorabfrage</option><option value=2>Aktor auf festen Wert setzen</option><option value=3>Aktor auf Sensorwert setzen</option>";	 
		else {
			if ( $mytype == 0 ) echo "<option value=0 selected>-???-</option>";		
			echo "<option value=1 ";
			if ( $mytype == 1 ) echo "selected";
			echo ">Sensorabfrage</option><option value=2 ";
			if ( $mytype == 2 ) echo "selected";
			echo ">Aktor auf festen Wert setzen</option><option value=3 ";
			if ( $mytype == 3 ) echo "selected";
			echo ">Aktor auf Sensorwert setzen</option>";	
		}
		echo "</select></td></tr>",
			 "<tr id=job-tr1 class=block2><td></td><td></td></tr>",
			 "<tr id=job-tr2 class=block2><td>Wert</td><td><input class=input id=value value='".$myvalue."'></td></tr>",
			 "<tr id=job-tr3 class=block2><td>Quelle</td><td>".$html_sensorid."</td></tr>";
	}	 
  echo "<tr><td colspan=2><button class=myButton onclick=updatejob('".$action."');>Speichern</button><button class=myButton onclick=updatejob('job_list');>Abbruch</button>",
       "&nbsp;</center></td></tr></table></center>";
}

function job_update($mycase, $myjob) {
	global $db;
	$my_error = 0;
	if (isset($_GET["jobname"])) {$myjobname=$_GET["jobname"];} else { $myjobname=""; }
	if (isset($_GET["addinfo"])) {$myaddinfo=$_GET["addinfo"];} else { $myaddinfo=""; }
	if (isset($_GET["type"])) {$mytype=$_GET["type"];} else { $mytype=0; }
	if (isset($_GET["id"])) {$myid=$_GET["id"];} else { $myid=0; }
	if (isset($_GET["value"])) {$myvalue=$_GET["value"];}
	if ( empty($myvalue)) $myvalue=0;
	if (isset($_GET["sensorid"])) {$mysensorid=$_GET["sensorid"];} else { $mysensorid=0; }
	$myjob_part=explode(".",$myjob);
	if ( $myjob_part[1] == "" ) {
		if ( $mycase == "savenew" ) $sql = "insert into job(job_id, job_name, add_info) values(".$myjob_part[0].", '".$myjobname."', '".$myaddinfo."')";
		if ( $mycase == "update" ) $sql = "update job set job_name = '".$myjobname."', add_info = '".$myaddinfo."' where job_id = ".$myjob_part[0];
		if ( $mycase == "delete" ) {
			$sql="select count(*) from jobstep where job_id = ".$myjob_part[0];
			$results = $db->query($sql);
			$row = $results->fetchArray();
			if ( $row[0] > 0 ) {
				mymessage("error","Fehler: Aktionen innerhalb der Jobkette vorhanden, bitte diese zuerst löschen!");
				$my_error=1;
			} else {
				$sql="delete from job where job_id = ".$myjob_part[0];
			}
		}
	} else {
		if ( $mycase == "savenew" ) $sql = "insert into jobstep(job_id, seq, add_info, type, ID, Value, sensor_id ) values(".$myjob_part[0].",".$myjob_part[1].", '".$myaddinfo."',".$mytype.", ".$myid.", ".$myvalue.", ".$mysensorid.")";
		if ( $mycase == "update" ) $sql = "update jobstep set add_info = '".$myaddinfo."', type = ".$mytype.", ID = ".$myid.", value = ".$myvalue.", sensor_id = ".$mysensorid." where job_id = ".$myjob_part[0]." and seq = ".$myjob_part[1];
		if ( $mycase == "delete" ) $sql = "delete from jobstep where job_id = ".$myjob_part[0]." and seq = ".$myjob_part[1];
	}
	if ($my_error == 0) {
		if ( $db->exec($sql) ) {
			mymessage("info","gespeichert");
		} else {
			mymessage("error","Fehler: SQL: $sql <br>".$db->lastErrorMsg());
		}
	}
}

function job_start_once($myjob) {
	global $db;
	$sql = "insert into scheduled_jobs(job_id) values ($myjob)";
	if ( $db->exec($sql) ) mymessage("info","Job $myjob gestartet");
}

function job_start_interval($myjob) {
	global $db;
	$sql = "insert into schedule(Schedule_ID,Job_ID,Triggered_By,Utime,Interval) values((select max(Schedule_ID)+1 from schedule),".$myjob.",'t',strftime('%s','now'),1200)";
	if ( $db->exec($sql) ) mymessage("info","Job $myjob gestartet");
}

#*************************************************************
#
# Schedule Funktionen
#
#*************************************************************

function schedule_list() {
	global $db;
	$sql = 	"select schedule_id, job_name, datetime(utime, 'unixepoch','localtime'), Interval/60, triggered_by, c.trigger_name, a.trigger_state ".
			" from schedule a, job b, trigger c where a.job_id = b.job_id and a.trigger_id = c.trigger_id and triggered_by = 'v' ".
			"union all ". 
			"select schedule_id, job_name, datetime(utime, 'unixepoch','localtime'), Interval/60, triggered_by, trigger_id, trigger_state ".
			" from schedule a, job b where a.job_id = b.job_id and triggered_by = 't'";
	echo "<center><table>";
	echo "<tr><th>Jobkette</th><th>Ausl&ouml;ser</th>",
		 "<th>Startzeitpunkt</th>",
		 "<th>Intervall<br>in Minuten</th><th>&nbsp;</th></tr>",
		 "<tr><th>&nbsp;</th><th>&nbsp;</th><th colspan=2>oder Trigger</th><th>&nbsp;</th></tr>";
	$results = $db->query($sql);
	while ($row = $results->fetchArray()) {
		echo "<tr class=block2><td>",$row[1],"</td>";
		if ($row[4]=="t") {
			echo "<td>Zeit</td><td>",$row[2],"</td><td>",$row[3],"</td>";
		} else {
			echo "<td>Trigger</td><td colspan=2>",$row[5];
			if ($row[6] == "s") echo " (auslösen)</td>";
			else echo " (rücksetzen)</td>";
		}
		echo "<td>",
			 "<button class=myButton onclick=getresult('schedule','schedule_edit','$row[0]');>Editieren</button>",
			 "<button class=myButton onclick=getresult('schedule','schedule_delete','$row[0]');>L&ouml;schen</button>",
			 "</td></tr>\n";
	}
	echo "<tr><td colspan=8 align=right>&nbsp;",
		 "<button class=myButton onclick=getresult('schedule','schedule_edit','0');>Neuen Ausf&uuml;hrungsplan anlegen</button>",
		 "&nbsp;</td></tr>",
		 "</table></center>\n";
}

function schedule_edit($myschedule) {
	global $db;
	$isnewschedule=FALSE;
	echo "<script> function updateschedule(myaction) { ",
		 "var myschedule=document.getElementById('schedule').value; var myjob=document.getElementById('job').value; ",
		 "var mytype=document.getElementById('type').value; var mytrigger=document.getElementById('trigger').value; ",
		 "var mystart=document.getElementById('start').value; var myinterval=document.getElementById('interval').value; ",
		 "var mystate=document.getElementById('state').value; ",
		 "$.get(myprogramm,{ display : 'schedule', action : myaction, dsid : myschedule, job : myjob, type : mytype, start : mystart, interval : myinterval, trigger : mytrigger, state : mystate}, function(data) { ",
		 "$('#result').hide(); $('#result').html(data); $('#result').show(); }); } </script>";
	if ($myschedule==0) {
		$results = $db->query("SELECT ifnull(max(schedule_id),0)+1 from schedule");
		$row = $results->fetchArray();
		$myschedule=$row[0];
		$myjob=0;
		$mytriggeredby="t";
		$mystart="";
		$myinterval="";
        $mytrigger=0;
		$isnewschedule=TRUE;
		$label1="Anlegen eines neuen Ausf&uuml;hrungsplans";
	} else {
		$results = $db->query("select job_id, Triggered_By, strftime('%Y-%m-%d %H:%M',datetime(Utime,'unixepoch', 'localtime')), Interval / 60, trigger_id, trigger_state from schedule where schedule_id = $myschedule ");
		$row = $results->fetchArray();
		$myjob=$row[0];
		$mytriggeredby=$row[1];
		$mystart=$row[2];
		$myinterval=$row[3];
		if ($row[4]=="") {$mytrigger=0; } else {$mytrigger=$row[4];}
		$s_sel="";
		$r_sel="";
		if ( $row[5] == "s" ) $s_sel = "selected"; else $r_sel = "selected"; 
		$label1="Editieren eines Ausführungsplanes";
	}
	echo "<center><table><tr><th colspan=2>$label1</th></tr>",
		 "<tr class=block2><td>Jobkette:</td><td>",
		 "<select name='job' id='job' size=1 class='input'>";
	if ( $myjob == 0 ) echo "<option value='' selected>---</option>";
	$sql="select job_id, job_name from job";
	$results = $db->query($sql);
	while ($row = $results->fetchArray()) {
		if ( $row[0] == $myjob ) $isselected="selected"; else $isselected="";
		echo "<option value='",$row[0],"' ",$isselected,">",$row[1],"</option>";
	}
	echo "</select></td></tr>",
		 "<tr class=block2><td>Ausl&ouml;ser:</td><td>",
         "<select name='type' id='type' size=1 class='input' onChange='hide_type_tr(this.value)'>";
	if ($mytriggeredby == "v" ) {
		echo "<option value='v' selected>Trigger</option><option value='t'>Zeit</option>";
	} else {
		echo "<option value='t' selected>Zeit</option><option value='v'>Trigger</option>";
	}
	echo "</td></tr>",
		 "<tr id=sch-tr1 class=block2><td>Startzeitpunkt Format:<br>YYYY-MM-DD HH:MM<br>oder -1 f&uuml;r sofort</td>",
		 "<td><input type=hidden id=schedule value=$myschedule><input id=start class=input value='$mystart'></td></tr>",
         "<tr id=sch-tr2 class=block2><td>Interval in Minuten<br>oder -1 f&uuml;r einmalig</td>",
		 "<td><input id=interval class=input value='$myinterval'></td></tr>",
		 "<tr id=sch-tr3 class=block2><td>Trigger:</td><td>",
		 "<select name='trigger' id='trigger' size=1 class='input'>";
	if ($mytrigger == 0) echo "<option value='0' selected>---</option>";
	$results = $db->query("select trigger_id, trigger_name from trigger ");
	while ($row = $results->fetchArray()) {
		if ( $row[0] == $mytrigger ) {
			echo "<option value=$row[0] selected>$row[1]</option>";
		} else {
			echo "<option value=$row[0] >$row[1]</option>";
		}
	}
	echo "</td></tr>",
		 "<tr id=sch-tr4 class=block2><td>Ereignis:</td><td>",
		 "<select name='state' id='state' size=1 class='input'>";
	echo "<option value='s' ".$s_sel.">Trigger auslösen</option><option value='r' ".$r_sel.">Trigger rücksetzen</option></select>$row[5]</td></tr>", 
		 "<tr><td colspan=2><center>";
	if ($isnewschedule) {
		echo "<button class=myButton onclick=updateschedule('schedule_savenew');>Werte eintragen</button>";
	} else {
		echo "<button class=myButton onclick=updateschedule('schedule_update');>Werte eintragen</button>";
	}
	echo "<button class=myButton onclick=updateschedule('schedule_list');>Abbruch</button>",
		 "&nbsp;</center></td></tr>",
		 "</table>",
		 "</center>",
		 "<script language=\"Javascript\">",
		 " function hide_type_tr(value) { ",
		 "   if ( value == 'v' ) {",
         "  \$('#sch-tr1').hide(); ",
         "  \$('#sch-tr2').hide(); ",
         "  \$('#sch-tr3').show(); ",
         "  \$('#sch-tr4').show(); ",
         " } else { ",
         "  \$('#sch-tr1').show(); ",
         "  \$('#sch-tr2').show(); ",
         "  \$('#sch-tr3').hide(); ",
         "  \$('#sch-tr4').hide(); ",
         " } ",
         "} ",
         "hide_type_tr('".$mytriggeredby."'); ",
         "</script>\n";
}

function schedule_update($mycase, $myschedule) {
	global $db;
	$my_error = 0;
	if ( ! ($mycase == "delete") ) {
		$myjob=$_GET["job"];
		$mytype=$_GET["type"];
		$mytrigger=$_GET["trigger"];
		$mystate=$_GET["state"];
		$mystart=$_GET["start"];
		$myinterval=$_GET["interval"];
	}
	switch($mycase) {
		case "update":
			if ($mytype == "t") {
				$sql = "update schedule set Job_ID = $myjob, Triggered_By = 't', utime = strftime('%s', datetime('".$mystart."'), 'utc'), interval = ".$myinterval."*60, trigger_id = 0, trigger_state = '-' where schedule_id = ".$myschedule;
			} else {
				$sql = "update schedule set Job_ID = $myjob, Triggered_By = 'v', utime = -1, interval = -1, trigger_id = ".$mytrigger.", trigger_state = '".$mystate."' where schedule_id = ".$myschedule;
			}
		break;
		case "savenew":
			if ($mytype == "t") {
				$sql="insert into schedule (schedule_id, Job_id, Triggered_By, utime, interval, trigger_id, trigger_state ) values( $myschedule, $myjob, 't', strftime('%s', datetime('".$mystart."'), 'utc'), ".$myinterval."*60, 0, '-' ) ";
			} else {
				$sql="insert into schedule (schedule_id, Job_id, Triggered_By, utime, interval, trigger_id, trigger_state ) values( $myschedule, $myjob, 'v', -1, -1, ".$mytrigger.", '".$mystate."' ) ";
			}			
		break;
		case "delete":
			$sql="delete from schedule where schedule_id = ".$myschedule; 
		break;
	}
	if ($my_error == 0) {
		if ( $db->exec($sql) ) {
			mymessage("info","gespeichert");
		} else {
			mymessage("error","Fehler: SQL: $sql <br>".$db->lastErrorMsg());
		}
	}
}

	
#*************************************************************
#
# Hauptfunktionen
#
#*************************************************************

echo "<script>  var myprogramm='/admin/sensorhubadmin.php'; function getresult(mydisplay, myaction, mydsid) { $.get(myprogramm,{ display : mydisplay, action : myaction, dsid : mydsid }, function(data) { $('#result').hide(); $('#result').html(data); $('#result').show(); }); } </script>";
echo "<style type='text/css'> td, th, tr, table { border:thin solid black; } </style>";

switch ($display) {
	case "":
		echo "<link rel='stylesheet' type='text/css' href='/css/styles.css'><p><center><h1>Sensorhub Administration</h1></center></p><div id='appmenu'><ul>",
			 "<li><a href='#' onclick=getresult('node','',0);>Nodes</a></li>",
			 "<li><a href='#' onclick=getresult('sensor','',0);>Sensoren</a></li>&nbsp;",
			 "<li><a href='#' onclick=getresult('actor','',0);>Aktoren</a></li>&nbsp;",
			 "<li><a href='#' onclick=getresult('trigger','',0);>Trigger</a></li>&nbsp;",
			 "<li><a href='#' onclick=getresult('job','',0);>Jobketten</a></li>&nbsp;",
			 "<li><a href='#' onclick=getresult('schedule','',0);>Ausf&uuml;hrungspl&auml;ne</a></li>&nbsp;",
			 "<style type='text/css'> td, th, tr, table { border:thin solid black; } </style>",
			 "</ul></div><div id='result'></div>";
	break;
	case "node":
		switch ($action) {
			case "node_edit":
				node_edit($dsid);
			break;
			case "node_delete":
				node_update("delete", $dsid);
				node_list();
			break;
			case "node_update":
				node_update("update",$dsid);
				node_list();
			break;
			case "node_savenew":
				node_update("savenew",$dsid);
				node_list();
			break;
			default:
				node_list();
		}
	break;
	case "sensor":
	case "actor":
		switch ($action) {
			case "sensor_edit":
			case "actor_edit":
				sensor_edit($display, $dsid);
			break;	
			case "sensor_delete":
			case "actor_delete":
				sensor_update($display, "delete", $dsid);
				sensor_list($display);
			break;
			case "sensor_update":
			case "actor_update":
				sensor_update($display, "update", $dsid);
				sensor_list($display);
			break;
			case "sensor_savenew":
			case "actor_savenew":
				sensor_update($display, "savenew", $dsid);
				sensor_list($display);
			break;				
			default:
				sensor_list($display);
		}
	break;
	case "trigger":
		switch ($action) {
			case "trigger_edit":
				trigger_edit($dsid);
			break;
			case "trigger_delete":
				trigger_update("delete", $dsid);
				trigger_list();
			break;
			case "trigger_update":
				trigger_update("update",$dsid);
				trigger_list();
			break;
			case "trigger_savenew":
				trigger_update("savenew",$dsid);
				trigger_list();
			break;
			default:
				trigger_list();
		}
	break;
	case "job":
		switch ($action) {
			case "job_edit":
				job_edit($dsid);
			break;
			case "job_delete":
				job_update("delete", $dsid);
				job_list();
			break;
			case "job_update":
				job_update("update",$dsid);
				job_list();
			break;
			case "job_savenew":
				job_update("savenew",$dsid);
				job_list();
			break;
			case "job_start_once":
				job_start_once($dsid);
				job_list();
			break;
			case "job_start_interval":
				job_start_interval($dsid);
				job_list();
			break;
			default:
				job_list();
		}
	break;
	case "schedule":
		switch ($action) {
			case "schedule_edit":
				schedule_edit($dsid);
			break;
			case "schedule_delete":
				schedule_update("delete", $dsid);
				schedule_list();
			break;
			case "schedule_update":
				schedule_update("update",$dsid);
				schedule_list();
			break;
			case "schedule_savenew":
				schedule_update("savenew",$dsid);
				schedule_list();
			break;
			default:
				schedule_list();
		}
	break;
	default:
		echo "Unbekannte Funktion: $display - $action";
}

$db->close();

?>










