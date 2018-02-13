use strict;
use warnings;


use IPC::SysV qw(IPC_PRIVATE S_IRUSR S_IWUSR);
use IPC::Msg;
use Class::Struct;

struct message => {
    mtype       => '$',
    name  	=> '$',
    value       => '$',
    prio        => '$',
};

my $mymsg = message->new(
    mtype        => pack("s",3),
    name         => "test",
    value        => 4.17,
    prio         => 4,
);

print "Test msg \n";
my $msg = IPC::Msg->new(53417, S_IRUSR | S_IWUSR);
my $msg_id = $msg->id;
my $mytype = 3;
my $myname = "Test";
print "Msg ID: $msg_id \n";
print  pack("aa30a20C",3,"test","4.1",4); print "#\n";
$msg->snd($msg_id, pack("l! a*",$mytype,$myname) ,0);
#$msg->snd($msgtype, $mymsg, 0);
#$msg->rcv($buf, 256);
#$ds = $msg->stat;
#$msg->remove;
