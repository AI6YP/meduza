#!/usr/bin/perl -w

use strict;
use Digest::CRC qw(crc crc16 crcccitt crcccitt_hex);

{
	die "usage: cleanex.pl logfile\n" unless ($#ARGV == 0);
	my $nname = $ARGV[0];
	open(INP, "<$nname") or die "Could not open file - input file: $!";
	binmode INP;
	my $stream;
	my $length = read (INP, $stream, 100000000);
	close(INP);

	my @STREAM = ();
	if (substr ($stream, 0, 2) eq 'FF') { # text mode
		for my $b (split (' ', $stream)) {
			push @STREAM, hex ($b);
		}
	} else { # binary mode
		for (my $i = 0; $i < $length; $i++) {
			push @STREAM, ord ( substr ($stream, $i, 1));
		}
	}
	shift @STREAM;
	shift @STREAM;

	my $state = 0;
	my $count = 0;
	my $block = 0;
	my $crc   = 0;
	my $bb;
	for my $b (@STREAM) {
		if ($state == 0) {
			if ($b == 255) {
				$count++;
				next;
			}
			if ($b == 254) {
				$state = 1;
				$count = 0;
				$crc = 0;
				$bb = '';
				next;
			}
			die "Sync lost with symbol $b after block \# $block\n";
		} else {
			$count++;
			$bb .= sprintf "%02X", $b;
			if ($count == 514) {
				$state = 0;
				$count = 0;
				$block++;
				my $a = pack ('H[1024]', $bb);
				$crc  = crcccitt ($a, 0x0000);
				if (substr ($bb, -4) eq sprintf ("%04X", $crc)) {
					print '^';
				} else {
					print '_';
				}
			}
		}
	}
	print "\n$block full blocks\n";
}
