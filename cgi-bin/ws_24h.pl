#!/usr/bin/perl -wT

use strict;

use CGI;
use GD;
use DBI;

my $dbh = DBI->connect("dbi:SQLite:/var/database/sensorhub.db", "", "", { RaiseError => 1 },) or die $DBI::errstr;
my $q     = new CGI;
my @hours=();
my $i=0;
my $j=0;
my $i_max=0;
my $delta_pix1=0;
my $delta_pix2=0;

my @titel=$q->param("titel");
my @sensor1=$q->param("sensor1");
my @sensor2=$q->param("sensor2");
my @reihe1=$q->param("reihe1");
my @reihe2=$q->param("reihe2");
my @val1=get_vals($sensor1[0]);
my @val2=get_vals($sensor2[0]);
my @minmax1=get_minmax(@val1);
my @minmax2=get_minmax(@val2);

print $q->header( -type => "image/png", -expires => "-1d" );
binmode STDOUT;
my $image = new GD::Image( 300, 370);
my $background = $image->colorAllocate( 255, 255, 255 );
my $val1_color = $image->colorAllocate( 255, 0, 0 );
my $val2_color = $image->colorAllocate( 0, 0, 255 );
my $axis_color = $image->colorAllocate( 0, 0, 0 );
my $coord_color = $image->colorAllocate( 200, 200, 200 );
my $text_color = $image->colorAllocate( 0, 0, 0 );
$image->transparent( $background );
 
# Add Title
$image->string( gdGiantFont, 120, 1, "Verlauf", $text_color );
$image->string( gdLargeFont, 60, 20, "der letzten 24 Stunden", $text_color );
$image->string( gdSmallFont, 120, 357, "Uhrzeit", $text_color );
$image->stringUp( gdSmallFont, 5, 150, $reihe1[0], $val1_color );
$image->stringUp( gdSmallFont, 270, 150, $reihe2[0], $val2_color );

# Draw Coord and add Axes
$image->filledRectangle(30,40,260,340,$coord_color);
$image->line( 30, 40, 30,340,$axis_color );
$image->line( 260,40,260,340,$axis_color );
$image->line( 30,340,260,340,$axis_color );
$image->line( 30, 40,260, 40,$axis_color );
$image->line( 30,190,260,190,$axis_color );

$image->string( gdSmallFont, 30-(length($minmax1[1])*8), 34, $minmax1[1], $val1_color );
$image->string( gdSmallFont, 30-(length(($minmax1[0]+$minmax1[1])/2)*8), 184, ($minmax1[0]+$minmax1[1])/2, $val1_color );
$image->string( gdSmallFont, 30-(length($minmax1[0])*8), 334, $minmax1[0], $val1_color );
$image->string( gdSmallFont, 265, 34,  $minmax2[1], $val2_color );
$image->string( gdSmallFont, 265, 184, ($minmax2[0]+$minmax2[1])/2, $val2_color );
$image->string( gdSmallFont, 265, 334, $minmax2[0], $val2_color );


$i=0;
$j=0;
my $x1;
my $x2;
$delta_pix1=300/($minmax1[1]-$minmax1[0]);
$delta_pix2=300/($minmax2[1]-$minmax2[0]);
while ( $i < $#val1 ) {
  $x1=260-(10*$i);
  $x2=250-(10*$i);
  $image->line($x1, 340-(($val1[$i]-$minmax1[0])*$delta_pix1), $x2, 340-(($val1[$i+1]-$minmax1[0])*$delta_pix1) ,$val1_color );
  $image->line($x1, 340-(($val2[$i]-$minmax2[0])*$delta_pix2), $x2, 340-(($val2[$i+1]-$minmax2[0])*$delta_pix2) ,$val2_color );
  if ($j == 1) { 
    $image->line( $x1, 336, $x1, 342 ,$axis_color );
    $image->string( gdSmallFont, $x1+3-(length($hours[$i])*4), 344, $hours[$i], $axis_color ); 
    $j=-1; 
  }
  $i++;
  $j++;
}

print $image->png;

# ----Subroutines----------------------------------------


sub get_vals {
  my $sensor=$_[0];
  my @row=();
  my @vals=();
  @hours=();
  my $sth = $dbh->prepare("SELECT Value, Year, Month, Day, Hour FROM sensordata where sensor= $sensor order by Year DESC, Month DESC, Day DESC, Hour DESC LIMIT 24");
  $sth->execute();
  while (@row = $sth->fetchrow_array()) {
    push(@vals,$row[0]);
    push(@hours,$row[4]);
  }
  $sth->finish();
  return @vals;
}

sub get_minmax {
  my @in=@_;
  my $max=$in[0];
  my $min=$in[0];
  my $i=0;
  while ($i <= $#in) {
    if ($max < $in[$i]) { $max=$in[$i]; }
    if ($min > $in[$i]) { $min=$in[$i]; }
    $i++;
  }
  return (sprintf("%.0f",($min-5)/10)*10,sprintf("%.0f",($max+5)/10)*10);
}
