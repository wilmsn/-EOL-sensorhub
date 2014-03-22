<script type="text/javascript">
  function test(myval) {
    alert(myval);
  }
  function getresult(mydsid, mydisplay, myaction) {
    $.get('/admin/sensorhubadmin.php',{ display : mydisplay, action : myaction, dsid : mydsid }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function updatenode() {
    var mynode=document.getElementById('node').value;
    var mynodename=document.getElementById('nodename').value;
    var mylocation=document.getElementById('location').value;
    var mybatteryun=document.getElementById('batteryun').value;
    var mybatteryue=document.getElementById('batteryue').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'node', action : 'update_node', dsid : mynode, nodename : mynodename, location : mylocation, batteryun : mybatteryun, batteryue : mybatteryue }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function savenewnode() {
    var mynode=document.getElementById('node').value;
    var mynodename=document.getElementById('nodename').value;
    var mylocation=document.getElementById('location').value;
    var mybatteryun=document.getElementById('batteryun').value;
    var mybatteryue=document.getElementById('batteryue').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'node', action : 'savenew_node', dsid : mynode, nodename : mynodename, location : mylocation, batteryun : mybatteryun, batteryue : mybatteryue }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function listnode() {
    $.get('/admin/sensorhubadmin.php',{ display : 'node' }, function(data){
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function updatesensor() {
    var mysensor=document.getElementById('sensor').value;
    var mysensorinfo=document.getElementById('sensorinfo').value;
    var mynode=document.getElementById('node').value;
    var mychannel=document.getElementById('channel').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'sensor', action : 'update_sensor', dsid : mysensor, sensorinfo : mysensorinfo, node : mynode, channel : mychannel}, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function savenewsensor() {
    var mysensor=document.getElementById('sensor').value;
    var mysensorinfo=document.getElementById('sensorinfo').value;
    var mynode=document.getElementById('node').value;
    var mychannel=document.getElementById('channel').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'sensor', action : 'savenew_sensor', dsid : mysensor, sensorinfo : mysensorinfo, node : mynode, channel : mychannel}, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function listsensor() {
    $.get('/admin/sensorhubadmin.php',{ display : 'sensor' }, function(data){
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function updateactor() {
    var myactor=document.getElementById('actor').value;
    var myactorinfo=document.getElementById('actorinfo').value;
    var mynode=document.getElementById('node').value;
    var mychannel=document.getElementById('channel').value;
    var mysource=document.getElementById('source').value;
    var myvalue=document.getElementById('value').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'actor', action : 'update_actor', dsid : myactor, actorinfo : myactorinfo, node : mynode, channel : mychannel, source : mysource, value : myvalue}, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function savenewactor() {
    var myactor=document.getElementById('actor').value;
    var myactorinfo=document.getElementById('actorinfo').value;
    var mynode=document.getElementById('node').value;
    var mychannel=document.getElementById('channel').value;
    var mysource=document.getElementById('source').value;
    var myvalue=document.getElementById('value').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'actor', action : 'savenew_actor', dsid : myactor, actorinfo : myactorinfo, node : mynode, channel : mychannel, source : mysource, value : myvalue}, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function listactor() {
    $.get('/admin/sensorhubadmin.php',{ display : 'actor' }, function(data){
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function listjob() {
    $.get('/admin/sensorhubadmin.php',{ display : 'job' }, function(data){
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function savenewjobchain() {
    var myjobdesc=document.getElementById('jobdesc').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'job', action : 'savenew_jobchain', jobdesc : myjobdesc }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function editjob(myjob, myseq) {
    $.get('/admin/sensorhubadmin.php',{ display : 'job', action : 'edit_job', dsid : myjob, seq : myseq }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function deletejob(myjob, myseq) {
    $.get('/admin/sensorhubadmin.php',{ display : 'job', action : 'delete_job', dsid : myjob, seq : myseq }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function updatejob() {
    var myjob=document.getElementById('job').value;
    var myseq=document.getElementById('seq').value;
    var myseqold=document.getElementById('seqold').value;
    var mysensor=document.getElementById('sensor').value;
    var myvalue=document.getElementById('value').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'job', action : 'update_job', dsid : myjob, sensor : mysensor, seq : myseq, seqold : myseqold, value : myvalue }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function savenewjob() {
    var myjob=document.getElementById('job').value;
    var myseq=document.getElementById('seq').value;
    var mysensor=document.getElementById('sensor').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'job', action : 'savenew_job', dsid : myjob, sensor : mysensor, seq : myseq }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function listschedule() {
    $.get('/admin/sensorhubadmin.php',{ display : 'schedule' }, function(data){
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function savenewschedule() {
    var myschedule=document.getElementById('schedule').value;
    var mystart=document.getElementById('start').value;
    var myinterval=document.getElementById('interval').value;
    var mytrigger=document.getElementById('trigger').value;
    var mytype=document.getElementById('type').value;
    var myjob=document.getElementById('job').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'schedule', action : 'savenew_schedule', dsid : myschedule, job : myjob,  start : mystart, interval : myinterval, trigger : mytrigger, type : mytype }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }
  function updateschedule() {
    var myschedule=document.getElementById('schedule').value;
    var mystart=document.getElementById('start').value;
    var myinterval=document.getElementById('interval').value;
    var mytrigger=document.getElementById('trigger').value;
    var mytype=document.getElementById('type').value;
    var myjob=document.getElementById('job').value;
    $.get('/admin/sensorhubadmin.php',{ display : 'schedule', action : 'update_schedule', dsid : myschedule, job : myjob, start : mystart, interval : myinterval, trigger : mytrigger, type : mytype }, function(data) {
      $('#result').hide();
      $('#result').html(data);
      $('#result').show();
    });
  }

  function actorsourceinput() {
    var mysource=document.getElementById('source').value;
    switch (mysource) {
	  case "v":
	    $('#actor_val_source').html("<td>Wert:</td><td><input class=input id=value ></td>");
	  break;
	  case "s":
	    $('#actor_val_source').html("<td>Sensornummer:</td><td><input class=input id=value ></td>");
	  break;
      case "-":
        alert("Bitte eine Quelle auswaehlen");
      break;		
	}
  }


  $(document).ready(function(){
    $('#nodes_e').click(function(){
      listnode();
    });
    $('#sensors_e').click(function(){
      listsensor();
    });
    $('#actors_e').click(function(){
      listactor();
    });
    $('#trigger_e').click(function(){
      $.get('/admin/sensorhubadmin.php',{ display : trigger});
    });
    $('#job_e').click(function(){
      listjob();
    });
    $('#schedule_e').click(function(){
      listschedule();
    });
  });
</script>