use strict;
use warnings;

use IPC::SysV qw(IPC_PRIVATE IPC_CREAT S_IRUSR S_IWUSR ftok);
use IPC::Msg;

struct message => {
    mtype        => '$',
    buffer_size  => '$',
    last_message => '$',
    buff         => '$',
};

my $key_in = ftok( "/home/joobeen/Desktop/learning", 'm' );
my ( $buffer ) = "";
my $buf_size   = 4000;
my $file       = shift @ARGV;
my $ifh;
my $is_stdin  = 0;
my $type_sent = 1;
my $last;

my $msg = pack('V V a4000 V', $mtype, $buffer_size, $buffer, $last);

if ( defined $file ) {
    open $ifh, "<", $file or die $!;
}
else {
    $ifh = *STDIN;
    $is_stdin++;
}

my $ipc_id = msgget( $key_in, IPC_CREAT | S_IRUSR | S_IWUSR );
my $msg = message->new(
    mtype        => 1,
    last_message => 0
);

print "\tbyte sent\tbuffer_size\tlast_message\n";

while ( <$ifh> ) {

    $last = read( $ifh, $buffer, $buf_size );

    $msg->buff( $buffer );
    $msg->buffer_size( $buf_size );

    if ( $last < $buf_size ) {
        $msg->last_message( 1 );
    }

    msgsnd( $ipc_id, pack( "l! a*", $type_sent, $msg ), 0 );

    print "\t", $last, "\t\t", $buf_size, "\t\t", $msg->last_message, "\n";

}

close $ifh unless $is_stdin;
