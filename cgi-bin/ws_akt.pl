#!/usr/bin/perl -w

use strict;
use CGI;
use GD;
use DBI;
 
my $dbh = DBI->connect("dbi:SQLite:/var/database/sensorhub.db", "", "", { RaiseError => 1 },) or die $DBI::errstr;
my $q     = new CGI;
my @sensor_nr=$q->param("sensor");
my @titel=$q->param("titel");
my @unit=$q->param("unit");
my @dezimal=$q->param("dezimal");


my $sth = $dbh->prepare("SELECT last_value FROM sensor where sensoradr = $sensor_nr[0] LIMIT 1");
$sth->execute();
my @row = $sth->fetchrow_array();
$sth->finish();

my $sprint_str="%.$dezimal[0]f";
my $out_num=sprintf($sprint_str, $row[0]);

my $title="";
my $unit="";
my $x_t=50-(length($titel[0])*4);
my $x_u=65;
my $x_v=60-(length($out_num)*10);

print $q->header( -type => "image/png", -expires => "-1d" );
binmode STDOUT;
my $image = new GD::Image( 100, 50);
my $background = $image->colorAllocate( 255, 255, 255 );
my $text_color = $image->colorAllocate( 0, 0, 0 );
                                          
# Add Title and Value
$image->string( gdLargeFont, $x_t, 2, $titel[0], $text_color );
$image->string( gdSmallFont, $x_u, 27, $unit[0], $text_color );
$image->string( gdGiantFont, $x_v, 25, $out_num, $text_color );
$image->rectangle(1,1,99,49, $text_color );

    
$image->transparent( $background );
    
print $image->png;
