<?php
#*******************************************************************
#
# Funktionen zur Pflege der Ausfuehrungsplaene
#
#*******************************************************************
function list_schedule() {
  global $db;
  global $tabledetails;
  echo "<center><table $tabledetails>";
  echo "<tr><th>Jobkette</th><th>Ausl&ouml;ser</th>",
       "<th>Startzeitpunkt</th>",
       "<th>Intervall<br>in Minuten</th><th>&nbsp;</th></tr>",
       "<tr><th>&nbsp;</th><th>&nbsp;</th><th colspan=2>oder Trigger</th><th>&nbsp;</th></tr>";
  $sql="select schedule, jobdesc, Start, Interval, type, c.triggername ".
       " from schedule a, jobchain b, trigger c where a.job = b.job and a.trigger = c.trigger and type = 'v' ".
       "union all ".
       "select schedule, jobdesc, Start, Interval, type, trigger ".
       " from schedule a, jobchain b where a.job = b.job and type = 't'";
  $results = $db->query($sql);
  while ($row = $results->fetchArray()) {
    echo "<tr class=block2><td>",$row[1],"</td>";
    if ($row[4]=="t") {
      echo "<td>Zeit</td><td>",$row[2],"</td><td>",$row[3],"</td>";
    } else {
      echo "<td>Trigger</td><td colspan=2>",$row[5],"</td>";
    }
    echo "<td>",
         "<button class=myButton value=$row[0] onclick=getresult(this.value,'schedule','edit_schedule');>Editieren</button>",
         "<button class=myButton value=$row[0] onclick=getresult(this.value,'schedule','delete_schedule');>L&ouml;schen</button>",
         "</td></tr>\n";
  }
  echo "<tr><td colspan=8 align=right>&nbsp;",
       "<button class=myButton value=0 onclick=getresult(this.value,'schedule','new_schedule');>Neuen Ausf&uuml;hrungsplan anlegen</button>",
       "&nbsp;</td></tr>",
       "</table></center>\n";
}

function edit_schedule($myschedule) {
  global $db;
  global $tabledetails_edit;
  if ($myschedule==0) {
    $mytype="t";
    $mystart="";
    $myinterval="";
    $mytrigger=0;
    echo "<center><table $tabledetails_edit><tr><th colspan=2>Anlegen eines neuen Ausf&uuml;hrungsplans</th></tr>",
         "<tr class=block2><td>Jobkette:</td><td>",
         "<select name='job' id='job' size=1 class='input'>",
         "<option value='' selected>---</option>";
    $sql="select job, jobdesc from  jobchain";
  } else {
    $results = $db->query("select schedule, job, Start, Interval, type, trigger from schedule where schedule = $myschedule ");
    $row = $results->fetchArray();
    $mytype=$row[4];
    $mystart=$row[2];
    $myinterval=$row[3];
    if ($row[5]=="") {$mytrigger=0; } else {$mytrigger=$row[5];}
    $results = $db->query("select b.job, b.jobdesc from  schedule a, jobchain b where a.job = b.job and a.schedule = $myschedule");
    $row = $results->fetchArray();
    echo "<center><table $tabledetails_edit><tr><th colspan=2>Editieren von Ausf&uuml;hrungsplan $myschedule</th></tr>",
         "<tr class=block2><td>Jobkette:</td><td>",
         "<select name='job' id='job' size=1 class='input'>",
         "<option value='row[0]' selected>$row[1]</option>";
    $sql="select job, jobdesc from  jobchain where job != $row[0]";
  }
  $results = $db->query($sql);
  while ($row = $results->fetchArray()) {
    echo "<option value='",$row[0],"'>",$row[1],"</option>";
  }
  echo "</select></td></tr>";
  echo "<tr class=block2><td>Ausl&ouml;ser:</td><td>",
         "<select name='type' id='type' size=1 class='input' onChange='hide_type_tr(this.value)'>";
  switch ($mytype) {
    case "v":
      echo "<option value='v' selected>Trigger</option>";
      echo "<option value='t'>Zeit</option>";
    break;
    default:
      echo "<option value='t' selected>Zeit</option>";
      echo "<option value='v'>Trigger</option>";
  }
  echo "</td></tr>",
       "<tr id=sch-tr1 class=block2><td>Startzeitpunkt Format:<br>YYYY-MM-DD HH:MM:SS<br>oder -1 f&uuml;r sofort</td>",
       "<td><input type=hidden id=schedule value=$myschedule><input id=start class=input value='$mystart'></td></tr>",
       "<tr id=sch-tr2 class=block2><td>Interval in Minuten<br>oder -1 f&uuml;r einmalig</td>",
       "<td><input id=interval class=input value='$myinterval'></td></tr>",
       "<tr id=sch-tr3 class=block2><td>Trigger:</td><td>",
       "<select name='trigger' id='trigger' size=1 class='input'>";
  if ($myschedule==0) {
    echo "<option value='0' selected>---</option>";
  } else {
    if ($mytrigger == 0) {
      echo "<option value='0' selected>---</option>";
    } else {
      $results = $db->query("select trigger, triggername from trigger where trigger = $mytrigger ");
      if ($row = $results->fetchArray()) {
        echo "<option value=$row[0] >$row[1]</option>";
      }
    }
  }
  $results = $db->query("select trigger, triggername from trigger where trigger != $mytrigger ");
  while ($row = $results->fetchArray()) {
    echo "<option value=$row[0] >$row[1]</option>";
  }
  echo "</td></tr>",
       "<tr><td colspan=2><center>";
  if ($myschedule==0) {
    echo "<button class=myButton onclick=savenewschedule();>Werte eintragen</button>";
  } else {
    echo "<button class=myButton onclick=updateschedule();>Werte eintragen</button>";
  }
  echo "<button class=myButton onclick=listschedule();>Abbruch</button>",
       "&nbsp;</center></td></tr>";
  echo "</table>",
       "</center>";
  echo "<script language=\"Javascript\">",
       " function hide_type_tr(value) { ",
        "   if ( value == 'v' ) {",
        "  \$('#sch-tr1').hide(); ",
        "  \$('#sch-tr2').hide(); ",
        "  \$('#sch-tr3').show(); ",
        " } else { ",
        "  \$('#sch-tr1').show(); ",
        "  \$('#sch-tr2').show(); ",
        "  \$('#sch-tr3').hide(); ",
        " } ",
        "} ",
        "hide_type_tr('$mytype'); ",
        "</script>\n";
}

function update_schedule($myschedule) {
  global $db;
  if (isset($_GET["type"]))      {$mytype=$_GET["type"];} else { $mytype=0; }
  if (isset($_GET["start"]))     {$mystart=$_GET["start"];} else { $mystart=" "; }
  if (isset($_GET["interval"]))  {$myinterval=$_GET["interval"];} else { $myinterval=1; }
  if (isset($_GET["trigger"]))   {$mytrigger=$_GET["trigger"];} else { $mytrigger=0; }
  $mytrigger=0;
  $sql="update schedule set type = '$mytype', interval = $myinterval, start = '$mystart', trigger = $mytrigger where schedule = $myschedule ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ".$db->lastErrorMsg()." <br>SQL: $sql");
  }
}

function savenew_schedule($myschedule) {
  global $db;
  if (isset($_GET["type"]))      {$mytype=$_GET["type"];} else { $mytype=0; }
  if (isset($_GET["start"]))     {$mystart=$_GET["start"];} else { $mystart=" "; }
  if (isset($_GET["interval"]))  {$myinterval=$_GET["interval"];} else { $myinterval=1; }
  if (isset($_GET["trigger"]))   {$mytrigger=$_GET["trigger"];} else { $mytrigger=0; }
  if (isset($_GET["job"]))       {$myjob=$_GET["job"];} else { $myjob=0; }
  $sql="insert into schedule (schedule, job, start, interval, type, trigger) values( (select ifnull(max(schedule),1)+1 from schedule), $myjob, '$mystart', $myinterval, '$mytype', $mytrigger) ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ".$db->lastErrorMsg()." <br>SQL: $sql");
  }
}

function delete_schedule($myschedule) {
  global $db;
  $sql="delete from schedule where schedule = $myschedule ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ".$db->lastErrorMsg()." <br>SQL: $sql");
  }
}
?>