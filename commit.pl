#!/usr/bin/perl
use strict;
use warnings;

if( !defined($ARGV[0]) or $ARGV[0] eq "--help" or $#ARGV > 0)
{
	print "usage: $0 \"commit description\"\n";
	exit(0);
}

# prepare all files
print "update ChangeLog...\n";
open (ChangeLog,">>","ChangeLog") or die "couldn't open ChangeLog\n";
print ChangeLog "* $ARGV[0]\n";
close ChangeLog;

print "indent source files...\n";
opendir(DIR, "./src") or die "couldn't open source directory\n";
my @sourcefiles = grep(/.[hc]^/, readdir(DIR));
closedir(DIR);

foreach my $file (@sourcefiles)
{
	system "indent ./src/$file";
}

print "commit...\n";
system "svn commit -m \"$ARGV[0]\"";

print "done.\n";
exit(0);
