#!/usr/bin/perl -w

use strict;
use Digest::CRC qw(crc crc16 crcccitt crcccitt_hex);

sub decode {
	my $in = shift;
	$in = sprintf "0x%02X", $in;

	my %T = (
		'0xFF' => 0,
		'0xFD' => 1,
		'0xFB' => 2,
		'0xF7' => 3,
		'0xEF' => 4,
		'0xDF' => 5,
		'0xBF' => 6,
		'0x7F' => 7,
		'0xF5' => 8,
		'0xEB' => 9,
		'0xED' => 10,
		'0xDD' => 11,
		'0xBB' => 12,
		'0x77' => 13,
		'0xB7' => 14,
		'0x5F' => 15
	);
	if (defined $T {$in}) {
#		print $in . '+';
	} else {
		print $in . '-';
	}
#	print $T {$in} . '=';
	return $T {$in};
}

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
	} elsif (ord (substr ($stream, 7, 1)) == 95) { # binary in irda mode
		for (my $i = 0; $i < $length; $i = $i + 4) {
			my $h;
			my $h2 = decode (ord (substr ($stream, $i + 2, 1)));
			my $h3 = decode (ord (substr ($stream, $i + 3, 1)));
			$h  = ($h2 + 16 * $h3);
#			if ($i < 2000) { printf "%02x ", $h; }
			push @STREAM, $h;

			my $h0 = decode (ord (substr ($stream, $i,     1)));
			my $h1 = decode (ord (substr ($stream, $i + 1, 1)));
			$h  = ($h0 + 16 * $h1);
#			if ($i < 2000) { printf "%02x ", $h; }
			push @STREAM, $h;
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
