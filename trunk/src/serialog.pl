#!/usr/bin/perl

use strict;
use Device::SerialPort;

{
	die "usage: serialog.pl logfile\n" unless ($#ARGV == 0);
	my $LOGFILE = $ARGV[0];
	my $PORT    = "/dev/ttyUSB0";           # port to watch

	my $ob = Device::SerialPort->new ($PORT) || die "Can't Open $PORT: $!";
	$ob->baudrate(4000000)   || die "failed setting baudrate";
	$ob->parity("none")    || die "failed setting parity";
	$ob->databits(8)       || die "failed setting databits";
	$ob->handshake("none") || die "failed setting handshake";
	$ob->write_settings    || die "no settings";

	open (LOG,">>${LOGFILE}") ||die "can't open smdr file $LOGFILE for append $!\n";
	open (DEV, "<$PORT") || die "Cannot open $PORT: $_";

	select(LOG), $| = 1;      # set nonbufferd mode

	while($_ = <DEV>){        # print input device to file
		print LOG $_;
	}

	undef $ob;
}
