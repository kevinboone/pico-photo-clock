# pico-photo-clock 

Kevin Boone, January 2023

## What is this?

`pico-photo-clock` is a desktop clock with photo slideshow backgrounds,
designed for use with the Waveshare 430x480 display and Waveshare real-time
clock module. These modules will stack nicely to make a self-contained
assembly. The real-time clock module keeps the
date and time when the hardware is powered off. No soldering is required to
construct the clock -- the parts all push together.

Images in JPEG format are read from an SD card build into the display module.
Connecting the Pico to a computer and running a terminal emulator provides a
command-line interface for setting the time, and basic troubleshooting.

The date/time display uses proper, anti-aliased fonts.

In principle, the program will work with other hardware. It needs:

- An display with a four-wire SPI interface to an ILI9488 panel controller
- A DS3231 I2C RTC module (these are all alike -- any DS3231 module should work)
- An SD card breakout adapter and, of course, an SD card.

If you're assembling your own hardware, the Pico pin assignments will almost
certainly need to be changed (in `conflg.h`).

The program randomizes the photo presentation and, by default, shows each for
three minutes. The date/time display is in the top-left corner, although this
can easily be changed.

Although it works as it stands, and it's telling the time on my office desk
right now, this programs is really intended to be the basis of more useful,
clock-type applications.  I think (hope) that the code is sufficiently well
organized to be readily extendable.

## Major limitations

pico-photo-clock does not handle daylight savings. If you're in a region that
respects daylight savings time, you'll need to change the time manually twice a
year.

There is, at present, no nice user interface for changing the time. You'll need
to connect to a computer and use a terminal emulator, to run 'set...' at the
prompt.

Image size: pico-photo-clock works best if given images that fit exactly on the
screen.  Smaller images will work, but will not fill the screen.  The Pico is
only barely adequate to the task of JPEG decompression and updating the
display; there's really no scope for additional computation to ensure that the
image is properly sized. The Pico certainly doesn't have resources to scale an
image, or trim it in a smart way. On a Linux system with ImageMagick installed,
the script `ppc-resize.pl` can be used to batch-convert images to be suitable
for pico-photo-clock. 

pico-photo-clock does not handle progressive JPEG format. There may be other
formats it can't handle -- the program has mostly been tested using JPEGs that
have been scaled/cropped using ImageMagick, so this utiliy's JPEGs definitely
work.

The upper limit on photo capacity is 65535, since each is stored with a 16-bit
index.  However, the program will run out of memory long before then.
pico-photo-clock definitely works with hundreds of photos, and probably will
work with thousands. However, as these files are scanned on start-up, using a
huge number will make the program slow to start.

The display is pretty slow to update -- the Pico is working at its limit here.

The clock/date display is drawn in its own box on the screen. It would be nice
if this text could be merged with the background image, but the meagre RAM of
the Pico makes this difficult-to-impossible to implement. 

## Setting the time

Connect the Pico's USB to a computer, and run a terminal emulator on the USB
port. On Linux you should be able to run:

   $ minicom -D /dev/ttyACM0

If you've used the Pico SDK, you've almost certainly set this up already.

Then enter

    set year month day hour minute second

at the prompt.

Enter "help" at the prompt for other commands that might be useful for
troubleshooting.

## Settings file 

pico-photo-clock reads a file 'ppc.rc' in the root directory of the SD
card. It doesn't matter if this file doesn't exist -- defaults will be
used. At present, only three properties are settable; here is an
example

    # Configuration file for pico-photo-clock
    mins_per_background_change=2
    clock_x=100
    clock_y=50

The clock x and y coordinates denote where the top-level corner of the
time/date display will be placed on the screen. The size of the clock with the
default fonts is 193x113, but the program won't allow the clock to be
positioned off the screen. So to put the clock on the bottom-right corner, you
could set both the x and y coordinates to 10000, or any other large number.

## Building

pico-photo-clock is designed to be built using the Pico C SDK. If you have the
SDK installed and working, building should be a matter of:

    PICO_SDK_PATH=...
    mkdir build
    cd build
    cmake ..
    make

## Sample images

For testing purposes, there are some JPEG sample image of the correct
size in the `jepgs/` directory. 

## More information

https://kevinboone.me/pico-photo-clock.html
     
## Legal and authors

pico-photo-clock is copyright(c) 2023 Kevin Boone, and released under the terms
of the GNU public licence, v3.0. However, it includes components from a number
of different people.  These components may have their own redistribution
stipulations. Please see the specific source modules for full details.  

The FAT filesystem implementation is by a person identified as ChaN, whose
real-life identity is unknown to me.

The JPEG decompressor is by Rich Geldreich and Chris Phoenix.

The LCD panel driver is based in part on examples provided by Waveshare
Electronics.

The SD card driver is written by me, Kevin Boone, but I referred extensively to
the work of Carl J Kugler.



