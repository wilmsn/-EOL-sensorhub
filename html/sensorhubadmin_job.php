<?php
#*******************************************************************
#
# Funktionen zur Pflege der Jobs / Jobketten
#
#*******************************************************************
function list_job() {
  global $db;
  global $tabledetails;
  if (isset($_GET["seq"]))    {$myseq=$_GET["seq"];} else { $myseq=0; }
  echo "<center><table $tabledetails>";
  echo "<tr class=block1><th>Jobkette</th><th>LfdNr</th><th>Sensor/Aktor</th><th>&nbsp;</th></tr>";
  $results = $db->query("SELECT job, Jobdesc from Jobchain");
  while ($row = $results->fetchArray()) {
    $myjob=$row[0];
    $results1 = $db->query("SELECT count(*)+1 from Job where job = $myjob ");
    $row1 = $results1->fetchArray();
    echo "<tr class=block2><td rowspan=$row1[0]>$row[1]</td>";
    $results1 = $db->query("SELECT  seq, Info from Job a, sensors_and_actors b where job = $myjob and a.type = b.type and a.id = b.id ");
    $i=0;
    while ($row1 = $results1->fetchArray()) {
      $myseq=$row1[0];
      if ( $i > 0) { echo "<tr class=block2>";}
      $i++;
      echo "<td>$myseq</td><td>",$row1[1],"</td>",
           "<td>",
           "<button class=myButton onclick=editjob(this.value,$myseq); value=$myjob>Editieren</button>",
           "<button class=myButton onclick=deletejob(this.value,$myseq); value=$myjob>L&ouml;schen</button>",
           "</td></tr>\n";
    }
    if ( $i > 0) {
      echo "<tr class=block2><td colspan=4 align=right>&nbsp;";
    } else {
      echo "<td colspan=4 align=right>&nbsp;",
           "<button class=myButton value=$row[0] onclick=getresult(this.value,'job','delete_jobchain');>Jobkette l&ouml;schen</button>&nbsp;";
    }
    echo "<button class=myButton value=$row[0] onclick=getresult(this.value,'job','new_job');>Neuen Job in dieser Jobkette anlegen</button>",
	     "</td></tr>",
		 "<tr><td colspan=4>&nbsp;",
		 "<button class=myButton value=$row[0] onclick=getresult(this.value,'job','start_job_once');>Jobkette sofort und einmalig starten</button>",
		 "&nbsp;",
		 "<button class=myButton value=$row[0] onclick=getresult(this.value,'job','start_job_interval');>Jobkette sofort mit Interval 10 min starten</button>",
		 "&nbsp;</td></tr>";
  }
  echo "<tr><td colspan=8 align=right>&nbsp;",
       "<button class=myButton value=0 onclick=getresult(this.value,'job','new_jobchain');>Neue Jobkette anlegen</button>",
       "&nbsp;</td></tr>",
       "</table></center>\n";
}

function edit_job($myjob) {
  global $db;
  global $tabledetails_edit;
  if (isset($_GET["seq"])) {$myseq=$_GET["seq"];} else { $myseq=0; }
  echo "<center><table $tabledetails_edit><tr><th colspan=2>";
  if ( $myseq == 0 ) {
    echo "Anlegen eines neuen Jobs";
    $mysensor="";
    $results = $db->query(" select ifnull(max(seq)+1,1) from job where job = $myjob ");
    $row = $results->fetchArray();
	$newseq=$row[0];
    $results = $db->query("select job, jobdesc from jobchain where job = $myjob ");
    $row = $results->fetchArray();
    echo "</th></tr><tr><td>Jobkette</td><td>$row[1]</td></tr>";
  } else {
    echo "Editieren eines Jobs";
    $newseq=$myseq;
    $results = $db->query("select a.key from sensors_and_actors a, job b where a.type = b.type and a.id = b.id and b.job = $myjob ");
    $row = $results->fetchArray();
    echo "</th></tr>";
  }
  echo "<tr class=block2><td>Sequenz:</td><td>",
       "<input type=hidden id=job value=$myjob>",
       "<input type=hidden id=seqold value=$myseq>",
       "<input id=seq class=input value=$newseq></td></tr>",
       "<tr class=block2><td>Sensor/Aktor</td><td>",
       "<select name='sensor' id='sensor' size=1 class='input'>";
  if ( $myseq == 0 ) {
    echo "<option value=0 selected>---</option>";
    $sql="SELECT key, info from sensors_and_actors "; 
  } else {
    $results = $db->query("SELECT key, info from sensors_and_actors a, job b where b.type = a.type and b.id = a.id and b.job = $myjob and b.seq = $myseq");
    $row = $results->fetchArray();
    echo "<option value=$row[0] selected>$row[1]</option>";
    $sql="SELECT key, info from sensors_and_actors where key <> '$row[0]'"; 
	}
  $results = $db->query($sql);
  while ($row = $results->fetchArray()) {
    echo "<option value='$row[0]'>$row[1]</option>";
  }
  echo "</select></td></tr><tr><td colspan=2>&nbsp;";
  if ( $myseq == 0 ) {
    echo "<button class=myButton onclick=savenewjob();>Werte eintragen</button>";
  } else {
    echo "<button class=myButton onclick=updatejob();>Werte eintragen</button>";
  }
  echo "<button class=myButton onclick=listjob();>Abbruch</button>",
       "&nbsp;</center></td></tr>";
  echo "</table>",
       "</center>";
}

function update_job($myjob) {
  global $db;
  $myseq=$_GET["seq"];
  $myseqold=$_GET["seqold"];
  $mysensor=$_GET["sensor"];
  $myvalue=$_GET["value"];
  $sql="update job set seq = $myseq, sensor = $mysensor, value = $myvalue where job = $myjob and seq = $myseqold ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ".$db->lastErrorMsg()." <br>SQL: $sql");
  }
}

function savenew_job($myjob) {
  global $db;
  $myseq=$_GET["seq"];
  $mysensor=$_GET["sensor"];
//  $sql="insert into job (job, seq, sensor, value) values( $myjob, $myseq, $mysensor, $myvalue ) ";
  $mytype=substr($mysensor,0,1);
  $myid=substr($mysensor,1);
  switch ($mytype) {
    case "s":
	  $sql="insert into job (job, seq, type, id) values( $myjob, $myseq, 1, $myid ) ";
	break;
    case "a":
	  $sql="insert into job (job, seq, type, id) values( $myjob, $myseq, 2, $myid ) ";
	break;
  }
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ".$db->lastErrorMsg()." <br>SQL: $sql");
  }
}

function delete_job($myjob) {
  global $db;
  $myseq=$_GET["seq"];
  $sql="delete from job where job = $myjob and seq = $myseq ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}

function new_jobchain() {
  echo "<center><table $tabledetails_edit><tr><th> Beschreibun der Jobkette </th></tr>",
       "<tr><td class=block2><input class=input id=jobdesc value=''></td></tr>",
       "<tr><td><button class=myButton onclick=savenewjobchain();>Jobkette anlegen</button></td></tr>",
       "</table>";
}

function savenew_jobchain() {
  global $db;
  $myjobdesc=$_GET["jobdesc"];
  $sql="insert into jobchain (job, jobdesc) values ( ( select ifnull(max(job),1)+1 from jobchain) , '$myjobdesc' )";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}

function delete_jobchain() {
  global $db;
  $mydsid=$_GET["dsid"];
  $sql="delete from jobchain where job = $mydsid ";
  if ( $db->exec($sql) ) {
    mymessage("info","gespeichert");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}

function start_job_interval() {
  global $db;
  $mydsid=$_GET["dsid"];
  $sql="insert into schedule(schedule,job,Start,Interval,type,trigger) values ( (select max(ifnull(schedule,0)+1) from schedule), $mydsid , '-1', 10, 't', 0)";
  if ( $db->exec($sql) ) {
    mymessage("info","Job gestartet");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}
function start_job_once() {
  global $db;
  $mydsid=$_GET["dsid"];
  $sql="insert into schedule(schedule,job,Start,Interval,type,trigger) values ( (select max(ifnull(schedule,0)+1) from schedule), $mydsid , '-1', '-1', 't', 0)";
  echo "SQL: $sql <br>";
  if ( $db->exec($sql) ) {
    mymessage("info","Job gestartet");
  } else {
    mymessage("error","Fehler: ",$db->lastErrorMsg()," <br>SQL: $sql");
  }
}

?>