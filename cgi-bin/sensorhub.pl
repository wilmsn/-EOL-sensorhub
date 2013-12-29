#!/usr/bin/perl -wT

use strict;
use CGI;
use DBI;

my $formparam=" action='/cgi-bin/admin/sensorhub.pl' method='get'";
my $dbname="/var/database/sensorhub.db";
my $tabledetails="width=1100 border=1 class='block1'";
my $tabledetails1="width=1050 border=1 class='block1'";

sub write_err {
  print "<table border=0><tr><td><img src=/img/stop.png></td>",
        "<td><span style=\"color:#FF0000;font-size:200%\">",
        "<strong>Kann Aktion nicht durchf&uuml;hren:<br>$_[0]</strong></span></td></tr></table></p>";
}

sub write_info {
  print "<table border=0><tr><td><img src=/img/info.png></td>",
        "<td><span style=\"color:#0000FF;font-size:200%\">",
        "<strong>Aktion durchgef&uuml;hrt:<br>$_[0]</strong></span></td></tr></table></p>";
}

sub td_input {
  return "<td>&nbsp;<input name='$_[0]' value='$_[1]' size='$_[2]' maxlength='$_[3]' class='$_[4]'>&nbsp;</td>\n";
}

sub td_input_select {
  my @myopt=@{$_[3]};
  my @myoptname=@{$_[4]};
  my $i=0;
  my $string;
  my $mystr="<td>&nbsp;<select name='$_[0]' size='1' class='$_[2]'>\n";
  if ($_[1] eq "") {
    $mystr=$mystr."<option value='-' selected>----</option>\n";
    foreach $string (@myopt) {
      $mystr=$mystr."<option value='$string'>$myoptname[$i]</option>\n";
      $i++;
    }
  } else {
    foreach $string (@myopt) {
      if ($string eq $_[1]){$mystr=$mystr."<option value='$string' selected>$myoptname[$i]</option>\n";}
      else {$mystr=$mystr."<option value='$string'>$myoptname[$i]</option>\n";}
      $i++;
    }
  }
  $mystr=$mystr."</select>&nbsp;</td>\n";
  return $mystr;
}


sub td_submit {
  return "<td>&nbsp;<input type=submit value='$_[0]' class='$_[1]'>&nbsp;</td>\n";
}

sub th {
  return "<th>&nbsp;$_[0]&nbsp;</th>\n";
}

sub start_form {
  return "<Form $formparam>\n<input type='hidden' name='action' value='$_[0]'>\n";
}

sub end_form {
  return "</form>\n";
}

my $q     = new CGI;
my $sth;
my $sql_stmt;
my @row;
my @row1;
my @nodes=();
my @nodenames=();
my $action=$q->param('action');
my $dbh = DBI->connect("dbi:SQLite:".$dbname, "", "", { RaiseError => 1 },) or die $DBI::errstr;
print $q->header( -type => "text/html", -expires => "-1d" );
print $q->start_html( -title => 'Sensorhub Administration');
print "<link rel=\"stylesheet\" type=\"text/css\" href=\"/css/styles.css\">";


#
# Action loop
#
if ($action eq "update_node") {
  $sql_stmt = "update NODE set Nodename = '" . $q->param('NODENAME') . "', Battery_UN = ". $q->param('UBATTS').
              ", Battery_UE = " . $q->param('UBATTE') . ", Location = '" . $q->param('LOC'). "' where node = '" . $q->param('NODE') . "'";
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Node ".$q->param('NODE')." aktualisiert! ");
} 
elsif($action eq "delete_node") {
  $sql_stmt = "select count(*) from SENSOR where node = '" . $q->param('NODE') . "'"; 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  (@row = $sth->fetchrow_array());
  if ( $row[0] > 0 ) {
    write_err("$row[0] Sensordatens&auml;tze vorhanden");
  } else {
    $sth->finish();
    $sql_stmt = "delete from NODE where node = '" . $q->param('NODE') . "'"; 
    $sth = $dbh->prepare($sql_stmt);
    $sth->execute();
    write_info("Node ".$q->param('NODE')." gel&ouml;scht! ");
  } 
  $sth->finish();
} 
elsif($action eq "new_node") {
  $sql_stmt = "insert into NODE (Node,Nodename,Battery_UN,Battery_UE,Location) values ('" .
              $q->param('NODE'). "', '".$q->param('NODENAME')."', ".$q->param('UBATTS').", ".$q->param('UBATTE'). ",'".$q->param('LOC')."')"; 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Node ".$q->param('NODE')." neu angelegt! ");
} 
elsif($action eq "update_sensor") {
  $sql_stmt = "update sensor set Sensorinfo = '" . $q->param('SENSORI') . "', Node = '". $q->param('NODE').
              "', Channel = " . $q->param('CHANNEL') . " where sensor = " . $q->param('SENSOR') . " ";
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Sensor ".$q->param('SENSOR')." aktualisiert! ");
}
elsif($action eq "delete_sensor") {
  $sql_stmt = "select count(*) from JOB where SENSOR = " . $q->param('SENSOR') . " "; 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  (@row = $sth->fetchrow_array());
  if ( $row[0] > 0 ) {
    write_err("$row[0] Jobdatens&auml;tze vorhanden");
  } else {
    $sth->finish();
    $sql_stmt = "delete from SENSOR where SENSOR = '" . $q->param('SENSOR') . "'"; 
    $sth = $dbh->prepare($sql_stmt);
    $sth->execute();
    write_info("Sensor ".$q->param('SENSOR')." gel&ouml;scht! ");
  } 
  $sth->finish();
} 
elsif($action eq "new_sensor") {
  $sql_stmt = "insert into SENSOR (Sensor, Sensorinfo, Node, Channel) values (" .
              $q->param('SENSOR'). ", '".$q->param('SENSORI')."', '".$q->param('NODE')."', ".$q->param('CHANNEL'). ")"; 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Sensor ".$q->param('SENSOR')." neu angelegt! ");
} 
elsif($action eq "update_jobchain") {
  $sql_stmt = "update jobchain set Jobdesc = '" . $q->param('JOBDESC') . "' where JOBNO = " . $q->param('JOBNO') . " ";
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Jobkette ".$q->param('JOBNO')." aktualisiert! ");
}
elsif($action eq "delete_jobchain") {
  $sql_stmt = "select count(*) from JOB where JOBNO = " . $q->param('JOBNO') . " "; 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  (@row = $sth->fetchrow_array());
  if ( $row[0] > 0 ) {
    write_err("$row[0] Jobdetaildatens&auml;tze vorhanden");
  } else {
    $sth->finish();
    $sql_stmt = "delete from SCHEDULE where JOBNO = '" . $q->param('JOBNO') . "'"; 
    $sth = $dbh->prepare($sql_stmt);
    $sth->execute();
    $sql_stmt = "delete from JOBCHAIN where JOBNO = '" . $q->param('JOBNO') . "'"; 
    $sth = $dbh->prepare($sql_stmt);
    $sth->execute();
    write_info("Jobkette ".$q->param('JOBNO')." gel&ouml;scht! ");
  } 
  $sth->finish();
} 
elsif($action eq "new_jobchain") {
  $sql_stmt = "insert into JOBCHAIN (JOBNO, JOBDESC) values (" .
              "(select ifnull(max(JOBNO)+1,1) from JOBCHAIN), '".$q->param('JOBDESC')."' )"; 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Neue Jobkette angelegt! ");
} 
elsif($action eq "update_job") {
  $sql_stmt = "update job set SENSOR = ". $q->param('SENSOR') .", VALUE = ". $q->param('VALUE') .
              "  where JOBNO = " . $q->param('JOBNO') . " and seq = " . $q->param('JOBSEQ');
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Job ".$q->param('JOBNO')."/".$q->param('JOBSEQ')." aktualisiert! ");
}
elsif($action eq "delete_job") {
  $sql_stmt = "delete from JOB where JOBNO = '" . $q->param('JOBNO') . "' and SEQ = " . $q->param('JOBSEQ'); 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute();
  write_info("Job ".$q->param('JOBNO')."/".$q->param('JOBSEQ')." gel&ouml;scht! ");
  $sth->finish();
} 
elsif($action eq "new_job") {
  $sql_stmt = "insert into JOB (JOBNO, SEQ, SENSOR, VALUE) values (" .
              $q->param('JOBNO'). ", (select ifnull(max(seq)+1,1) from job where jobno=".$q->param('JOBNO')."), ".$q->param('SENSOR').", ".$q->param('VALUE')." )"; 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Neuer Job in Jobkette ".$q->param('JOBNO')." angelegt! ");
} 
elsif($action eq "update_schedule") {
  $sql_stmt = "update schedule set JOBNO = " . $q->param('JOBNO') . ", start = '" . $q->param('START') . "', INTERVAL = ". $q->param('INTERVAL') .
              "  where SCHEDULE = " . $q->param('SCHEDULE') . " ";
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Ausf&uuml;hrungsplan ".$q->param('SCHEDULE')."  aktualisiert! ");
}
elsif($action eq "delete_schedule") {
  $sql_stmt = "delete from SCHEDULE where SCHEDULE = " . $q->param('SCHEDULE'); 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute();
  write_info("Ausf&uuml;hrungsplan ".$q->param('SCHEDULE')." gel&ouml;scht! ");
  $sth->finish();
} 
elsif($action eq "new_schedule") {
  $sql_stmt = "insert into SCHEDULE (SCHEDULE, JOBNO, START, INTERVAL) values (" .
              "(select ifnull(max(schedule)+1,1) from schedule), ".$q->param('JOBNO'). ", '".$q->param('START')."', ".$q->param('INTERVAL')." )"; 
  $sth = $dbh->prepare($sql_stmt);
  $sth->execute(); 
  $sth->finish();
  write_info("Neuer Ausf&uuml;hrungsplan angelegt! ");
} 
else {
#  default_statement";
}




#
# Ende Action loop
#

print "<h1>Nodes bearbeiten</h1>\n";

$sth = $dbh->prepare("SELECT Node, Nodename, Battery_UN, ifnull(Battery_act,\"---\"), Battery_UE, Location FROM node");
print "<Table $tabledetails><tr>",
      th("Node"), th("Nodename"), th("Batterie<br>Soll-<br>spannung"), th("Batterie<br>Ist-<br>spannung"), th("Batterie<br>Grenz-<br>spannung"), th("Standort"), th(" "), th(" ");
$sth->execute(); 
while (@row = $sth->fetchrow_array()) {
  print start_form("update_node"),
        "<tr class=\"block2\">",
        td_input("NODE",$row[0],5,5,"inputro"),
        td_input("NODENAME",$row[1],10,50,"input"),
        td_input("UBATTS",$row[2],5,5,"input"),
        td_input("UBATTI",$row[3],5,5,"inputro"),
        td_input("UBATTE",$row[4],5,5,"input"),
        td_input("LOC",$row[5],30,80,"input"),
        td_submit("Editieren","myButton"),
        end_form(),
        start_form("delete_node"),
        "<input type=hidden name='NODE' value='$row[0]'>",
        td_submit("Löschen","myButton"),
        end_form(),
        "</tr>"; 
  push(@nodes,$row[0]);     
  push(@nodenames,$row[1]);     
}
$sth->finish();
print start_form("new_node"),
      "<tr class=\"block2\">",
      td_input("NODE","",5,5,"input"),
      td_input("NODENAME","",10,50,"input"),
      td_input("UBATTS","",5,5,"input"),
      td_input("UBATTI","",5,5,"inputro"),
      td_input("UBATTE","",5,5,"input"),
      td_input("LOC","",30,80,"input"),
      td_submit("Anlegen","myButton"),
      "<td>&nbsp;</td>",
      end_form(),
      "</tr>",
      "</table>"; 

print "<h1>Sensoren bearbeiten</h1>\n";
my @sensorno=();
my @sensorname=();
$sth = $dbh->prepare("SELECT Sensor, Sensorinfo, Node, Channel, ifnull(Last_Value,\"---\"), ifnull(Last_TS,\"---\") FROM sensor");
print "<Table $tabledetails><tr>",
      th("Sensor"), th("Sensor<br>Beschreibung"), th("Node"), th("Channel"), th("Letzter<br>Wert"), th("letzte<br>Abfrage"), th(" "), th(" ");
$sth->execute(); 
while (@row = $sth->fetchrow_array()) {
  print start_form("update_sensor"),
        "<tr class=\"block2\">",
        td_input("SENSOR",$row[0],5,5,"inputro"),
        td_input("SENSORI",$row[1],30,80,"input"),
        td_input_select("NODE",$row[2],"input",\@nodes,\@nodenames),
        td_input("CHANNEL",$row[3],3,3,"input"),
        td_input("LASTVAL",$row[4],5,10,"inputro"),
        td_input("LASTTS",$row[5],15,15,"inputro"),
        td_submit("Editieren","myButton"),
        end_form(),
        start_form("delete_sensor"),
        "<input type=hidden name='SENSOR' value='$row[0]'>",
        td_submit("Löschen","myButton"),
        end_form(),
        "</tr>"; 
  push(@sensorno,$row[0]);     
  push(@sensorname,$row[1]);     
}
$sth->finish();
print start_form("new_sensor"),
      "<tr class=\"block2\">",
      td_input("SENSOR"," ",5,5,"input"),
      td_input("SENSORI"," ",30,80,"input"),
      td_input_select("NODE","","input",\@nodes,\@nodenames),
      td_input("CHANNEL"," ",3,3,"input"),
      td_input("LASTVAL"," ",5,10,"inputro"),
      td_input("LASTTS"," ",15,15,"inputro"),
      td_submit("Anlegen","myButton"),
      "<td>&nbsp;</td>",
      end_form(),
      "</tr>",
      "</table>"; 


print "<h1>Jobketten bearbeiten</h1>\n";
my @jobno=();
my @jobname=();

$sth = $dbh->prepare("SELECT jobno, jobdesc FROM jobchain");
$sth->execute(); 
while (@row = $sth->fetchrow_array()) {
  print "<Table $tabledetails><tr>\n",
        th("Jobketten<br>Beschreibung"), th(" "), th(" "),
        start_form("update_jobchain"),
        "<tr class=\"block2\">\n",
        "<input type=hidden name='JOBNO' value='$row[0]'>\n",
        td_input("JOBDESC",$row[1],80,120,"input"),
        td_submit("Editieren","myButton"),
        end_form(),
        start_form("delete_jobchain"),
        "<input type=hidden name='JOBNO' value='$row[0]'>\n",
        td_submit("Löschen","myButton"),
        end_form(),
        "</tr>\n"; 
  push(@jobno,$row[0]);     
  push(@jobname,$row[1]);     
  my $sth1 = $dbh->prepare("SELECT jobno, seq, sensor, value FROM job where Jobno = $row[0] order by seq ");
  print "<tr class=\"block1\"><td colspan=4>",
        "<Table $tabledetails1><tr>",
        th("Sensor"), th("Value"), th(" "), th(" ");
  $sth1->execute(); 
  while (@row1 = $sth1->fetchrow_array()) {
    print start_form("update_job"),
          "<input type=hidden name='JOBNO' value='$row1[0]'>\n",
          "<input type=hidden name='JOBSEQ' value='$row1[1]'>\n",
          "<tr class=\"block1\">\n",
          td_input_select("SENSOR",$row1[2],"input",\@sensorno,\@sensorname),
          td_input("VALUE",$row1[3],10,10,"input"),
          td_submit("Editieren","myButton"),
          end_form(),
          start_form("delete_job"),
          "<input type=hidden name='JOBNO' value='$row[0]'>\n",
          "<input type=hidden name='JOBSEQ' value='$row1[1]'>\n",
          td_submit("Löschen","myButton"),
          end_form(),
          "</tr>\n"; 
  }
  $sth1->finish();
  print start_form("new_job"),
        "<input type=hidden name='JOBNO' value='$row[0]'>",
        "<tr class=\"block1\">",
        td_input_select("SENSOR","","input",\@sensorno,\@sensorname),
        td_input("VALUE","0",10,10,"input"),
        td_submit("Anlegen","myButton"),
        "<td>&nbsp;</td>",
        end_form(),
        "</tr>",
        "</table></tr></table>\n"; 
}
$sth->finish();
print "<Table $tabledetails><tr>\n",
      th("Jobkette"), th("Jobketten<br>Beschreibung"), th(" "),
      start_form("new_jobchain"),
      "<tr class=\"block2\">\n",
      td_input("JOBDESC"," ",80,120,"input"),
      td_submit("Anlegen","myButton"),
      end_form(),
      "</tr>",
      "</table>\n";

print "<h1>Ausf&uuml;hrungspl&auml;ne bearbeiten</h1>\n";

$sth = $dbh->prepare("SELECT schedule, jobno, Start, Interval FROM schedule");
print "<Table $tabledetails><tr>",
      th("Jobkette"), th("Startzeitpunkt<br>Format:<br>YYYY-MM-DD HH:MM:SS<br>oder -1 f&uuml;r sofort"),
      th("Intervall<br>in Minuten<br>oder -1 f&uuml;r einmalig"), th(" "), th(" ");
$sth->execute(); 
while (@row = $sth->fetchrow_array()) {
  print start_form("update_schedule"),
        "<tr class=\"block2\">",
        "<input type=hidden name='SCHEDULE' value='$row[0]'>",
        td_input_select("JOBNO",$row[1],"input",\@jobno,\@jobname),
        td_input("START",$row[2],20,20,"input"),
        td_input("INTERVAL",$row[3],10,10,"input"),
        td_submit("Editieren","myButton"),
        end_form(),
        start_form("delete_schedule"),
        "<input type=hidden name='SCHEDULE' value='$row[0]'>",
        td_submit("Löschen","myButton"),
        end_form(),
        "</tr>"; 
}
$sth->finish();
print start_form("new_schedule"),
      "<tr class=\"block2\">",
      td_input_select("JOBNO","","input",\@jobno,\@jobname),
      td_input("START"," ",20,20,"input"),
      td_input("INTERVAL"," ",10,10,"input"),
      td_submit("Anlegen","myButton"),
      end_form(),
      "</tr>",
      "</table>"; 

print "<h1>&nbsp;</h1><center>",
      start_form("reload"),
      td_submit("Seite neu laden","myButtonBig"),
      end_form(),
      "</center></html>";


