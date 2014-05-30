<?php
if (isset($_GET["action"]))  {$action=$_GET["action"];} else { $action=""; }
if (isset($_GET["display"])) {$display=$_GET["display"];} else { $display=""; }
if (isset($_GET["dsid"]))    {$dsid=$_GET["dsid"];} else { $dsid=0; }
# $window_innerwidth=$_GET["window_innerwidth"];
$tabledetails="border=1 class=block1";
$tabledetails_edit="border=1 class=block1";
$DB_FILENAME="/var/database/sensorhub.db";
$db = new SQLite3($DB_FILENAME);
# echo $_SERVER['HTTP_USER_AGENT'] . "\n\n";
#*************************************************************
#
# Allgemeine Funktionen
#
#*************************************************************

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

switch ($display) {
  case "":
    echo "<script src='http://ajax.googleapis.com/ajax/libs/jquery/1.10.2/jquery.min.js'></script>";
    echo "<script src='sensorhub.js'></script>";
    include('sensorhubmenu.html');
  break;
  case "sensor":
    include('sensorhubadmin_sensor.php');
    switch ($action) {
      case "":
        list_sensor();
      break;
      case "edit_sensor":
        edit_sensor($dsid);
      break;
      case "new_sensor":
        edit_sensor(0);
      break;
      case "delete_sensor":
        delete_sensor($dsid);
        list_sensor();
      break;
      case "update_sensor":
        update_sensor($dsid);
        list_sensor();
      break;
      case "savenew_sensor":
        savenew_sensor($dsid);
        list_sensor();
      break;
    }
  break;
  case "actor":
    include('sensorhubadmin_actor.php');
    switch ($action) {
      case "":
        list_actor();
      break;
      case "edit_actor":
        edit_actor($dsid);
      break;
      case "new_actor":
        edit_actor(0);
      break;
      case "delete_actor":
        delete_actor($dsid);
        list_actor();
      break;
      case "update_actor":
        update_actor($dsid);
        list_actor();
      break;
      case "savenew_actor":
        savenew_actor($dsid);
        list_actor();
      break;
    }
  break;
  case "node":
    include('sensorhubadmin_node.php');
    switch ($action) {
      case "":
        list_node();
      break;
      case "edit_node":
        edit_node($dsid);
      break;
      case "new_node":
        edit_node(0);
      break;
      case "delete_node":
        delete_node($dsid);
        list_node();
      break;
      case "update_node":
        update_node($dsid);
        list_node();
      break;
      case "savenew_node":
        savenew_node($dsid);
        list_node();
      break;
    }
  break;
  case "job":
    include('sensorhubadmin_job.php');
    switch ($action) {
      case "":
        list_job();
      break;
      case "edit_job":
        edit_job($dsid);
      break;
      case "new_job":
        edit_job($dsid);
      break;
      case "new_jobchain":
        new_jobchain();
      break;
      case "savenew_jobchain":
        savenew_jobchain();
        list_job();
      break;
      case "delete_jobchain":
        delete_jobchain();
        list_job();
      break;
      case "delete_job":
        delete_job($dsid);
        list_job();
      break;
      case "update_job":
        update_job($dsid);
        list_job();
      break;
      case "savenew_job":
        savenew_job($dsid);
        list_job();
      break;
      case "start_job_once":
        start_job_once($dsid);
        list_job();
      break;
      case "start_job_interval":
        start_job_interval($dsid);
        list_job();
      break;
    }
  break;
  case "schedule":
    include('sensorhubadmin_schedule.php');
    switch ($action) {
      case "":
        list_schedule();
      break;
      case "edit_schedule":
        edit_schedule($dsid);
      break;
      case "new_schedule":
        edit_schedule(0);
      break;
      case "delete_schedule":
        delete_schedule($dsid);
        list_schedule();
      break;
      case "update_schedule":
        update_schedule($dsid);
        list_schedule();
      break;
      case "savenew_schedule":
        savenew_schedule($dsid);
        list_schedule();
      break;
    }
  break;
}

$db->close();

?>