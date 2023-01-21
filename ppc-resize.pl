#!/usr/bin/perl -w
# 
# ppc-resize.pl
# 
# Usage: ppc-resize.pl {files...} {directory}
#
# Uses ImageMagick to convert a set of files to a suitable format to be
#   displayed on pico-photo-clock. The converted files are written to the
#   specified directory -- which must exist -- keeping their original
#   names.
# Only files in landscape orientation are converted, unless 'keep_portrait'
#   is set to non-zero. 
#
# Copyright (c)2023 Kevin Boone, GPL v3.0

use strict;
use File::Basename;

my $argc = scalar (@ARGV);
my $keep_portrait = 0;

if ($argc < 2)
  {
  die "Usage: ppc-resize.pl {files...} {directory}\n";
  }

my $output_dir = $ARGV[$argc - 1];

printf ("dir=$output_dir\n");

if (not (-d $output_dir)) 
  {
  die "$output_dir is not a directory\n";
  }

for (my $i = 0; $i < $argc - 1; $i++)
  {
  my $arg = $ARGV[$i];
  my ($filename, $dir, $dummy) = fileparse ($arg, ""); 
  my $fileinfo = `file \"$arg\"`;
  $fileinfo =~ /, (\d+)x(\d+)/;
  my $width=$1;
  my $height=$2;
  if (($width > $height) or $keep_portrait)
    {
    printf ("${filename} is %dx%d\n", $width, $height);
    my $cmd = 
      sprintf ("convert \"%s\" -resize 480x320 $output_dir/%s", $arg, $filename); 
    system ($cmd);
    }
  else
    {
    printf ("Skipping ${filename}: portrait layout\n");
    }
  }

