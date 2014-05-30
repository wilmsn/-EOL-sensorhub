<?php
#*******************************************************************
#
# Funktionen zur Aktorpflege
#
#*******************************************************************

function input_select_node($mynode) {
  global $db;
  echo "<select name='node' id='node' size=1 class='input'>";
  if ($mynode == "") { echo "<option value='' selected>---</option>"; }
  $results = $db->query("SELECT Node, Nodename from node");
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

function list_actor() {
  global $db;
  global $tabledetails;
  echo "<center><table $tabledetails>";
  echo "<tr><th>Actor</th><th>Node</th><th>Channel</th><th>Quelle</th><th>Inhalt</th><th>&nbsp;</th></tr>";
  $results = $db->query("SELECT Actor, Actorinfo, Node, Channel, Source, Value ".
                       " ,julianday(date('now')) - ifnull((select julianday(date(max(last_ts))) from sensor where node = a.node),1) ".
                        "from Actor a ".
						"order by substr(Node,length(node),1), substr(Node,length(node)-1,1), substr(Node,length(node)-2,1), channel  ");
  while ($row = $results->fetchArray()) {
    if ($row[6] > 1) { $td_col = "bgcolor=#999999"; } else { $td_col = ""; }
    echo "<tr class=block2><td $td_col>",$row[1],"</td><td $td_col align=right>",$row[2],"</td><td $td_col>",$row[3],"</td>";
	switch ($row[4]) {
	  case "s":
	    echo "<td $td_col>Sensor</td><td $td_col>",$row[5],"</td><td $td_col>";
		break;
	  case "v":
	    echo "<td $td_col>fester Wert</td><td $td_col>",$row[5],"</td><td $td_col>";
		break;
	}
    echo "<button class=myButton value=$row[0] onclick=getresult(this.value,'actor','edit_actor');>Editieren</button>",
         "<button class=myButton value=$row[0] onclick=getresult(this.value,'actor','delete_actor');>L&ouml;schen</button>",
         "</td></tr>\n";
  }
  echo "<tr><td colspan=8 align=right>&nbsp;",
       "<button class=myButton value=0 onclick=getresult(this.value,'actor','new_actor');>Neuen Aktor anlegen</button>",
       "&nbsp;</td></tr>",
       "</table></center>\n";
}

function edit_actor($myactor) {
  global $db;
  global $tabledetails_edit;
  echo "<center><table $tabledetails_edit><tr><th colspan=2>";
  if ($myactor==0) {
    $results = $db->query("SELECT ifnull(max(Actor),0)+1 from Actor");
    $row = $results->fetchArray();
	$myactor=$row[0];
    $myactorinfo="";
    $mynode="";
    $mychannel="";
	$actor_is_new=1;
    echo "Anlegen eines neuen Aktors ($myactor)";
  } else {
    $results = $db->query("SELECT Actor, Actorinfo, Node, Channel, Source, Value from Actor where Actor = $myactor ");
    $row = $results->fetchArray();
    $myactorinfo=$row[1];
    $mynode=$row[2];
    $mychannel=$row[3];
	$actor_is_new=0;
    echo "Editieren eines Aktors ($myactor)";
  }
  echo 	"</th></tr>",
        "<tr class=block2><td>Aktorname:</td><td>",
        "<input type=hidden id=actor value=$row[0]>",
		"<input class=input id=actorinfo value='",$myactorinfo,"'>","</td></tr>",
       "<tr><td>Node:</td><td>";
  input_select_node($mynode);
  echo "</td></tr>",
       "<tr><td>Channel:</td><td><input class=input id=channel value='",$mychannel,"'>","</td></tr>",
	   "<tr><td>Quelle:</td><td><select size=1 id='source' class=input onchange='actorsourceinput()'>";
  if ($actor_is_new==1) {   
	echo "<option value='-' selected>---</option><option value='s'>Sensor</option><option value='v'>fester Wert</option></select></td></tr>",
	     "<tr id='actor_val_source'><td></td><td></td></tr>";
  } else {
    switch ($row[4]) {
	  case "s":
	    echo "<option value='s' selected>Sensor</option><option value='v'>fester Wert</option></select></td></tr>",
	     "<tr id='actor_val_source'><td>Sensornummer:</td><td><input class=input id=value value='$row[5]'></td></tr>";
	  break;	 
	  case "v":
	    echo "<option value='s'>Sensor</option><option value='v' selected>fester Wert</option></select></td></tr>",
	     "<tr id='actor_val_source'><td>Wert:</td><td><input class=input id=value value='$row[5]'></td></tr>";
	  break;
      default:
        echo "<tr><td>$row[4]</td><td>$row[5]</td></tr>";	  
	}
  }
  echo "<tr><td colspan=2><center>&nbsp;";  
  if ($actor_is_new==1) {
    echo "<button class=myButton onclick=savenewactor();>Aktor anlegen</button>";
  } else {
    echo "<button class=myButton onclick=updateactor();>Werte eintragen</button>";
  }
  echo "<button class=myButton onclick=listactor();>Abbruch</button>",
       "&nbsp;</center></td></tr>";
  echo "</table>",
       "</center>";
}

function update_actor($myactor) {
  global $db;
  $myactorinfo=$_GET["actorinfo"];
  $mynode=$_GET["node"];
  $mychannel=$_GET["channel"];
  $mysource=$_GET["source"];
  $myvalue=$_GET["value"];
  $sql="update actor set node = '$mynode', channel = $mychannel, actorinfo = '$myactorinfo', source = '$mysource', value = $myvalue where actor = $myactor ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}

function savenew_actor($myactor) {
  global $db;
  $myactorinfo=$_GET["actorinfo"];
  $mynode=$_GET["node"];
  $mychannel=$_GET["channel"];
  $mysource=$_GET["source"];
  $myvalue=$_GET["value"];
  $sql="insert into actor (actor, actorinfo, node, channel, source, value) values( $myactor, '$myactorinfo', '$mynode', $mychannel, '$mysource', $myvalue) ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}

function delete_actor($myactor) {
  global $db;
  $sql="select node, channel from actor where actor = $myactor ";
  $results = $db->query($sql);
  $row = $results->fetchArray();
  $mynode=$row[0];
  $mychannel=$row[1];
  $sql="select count(*) from job where type = 2 and id = $myactor ";
  $results = $db->query($sql);
  $row = $results->fetchArray();
  if ( $row[0] > 0 ) {
      mymessage("error","Fehler: Jobdatensätze vorhanden, dieser Aktor kann nicht gelöscht werden!");
  } else {
    $sql="delete from actor where actor = $myactor ";
    if ( $db->exec($sql) ) {
      mymessage("info","gespeichert");
    } else {
      mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
    }
  }
}
?>