Another Linux File Commander
============================

ALFC
by Stu George


What is it?
===========
ALFC is a clone of a famous old DOS program called Norton Commander.
Written under the GPL v2 license.


ALFC embeds the Lua scripting language to give you a lot of power
and flexability in your system management.


Design Spec
- Dont over-engineer
- Not supposed to supplant vi/emacs/foo as programming editor
- Is supposed to sustain normal admin tasks (file management, config file editing, etc).



Moving Around ALFC
==================


Current speed keyed commands
============================

:q - Quit app
:s - Make inactive window pane the same path as the active pane.
:c - cd
	:c ~/foo/bar
	:c $HOME/Documents
:j - Jump
	:j PinkShirt - Finds the closest match to PinkShirt in the list.... which might
	match Pink or even just P....
:f - Filter window (clear all existing filters)
	:f flv$ - show all files that end in flv
:f+ - Add to filter (posix regex)
	:f+ \.[ch] - Show all c header and source files
:g - Glob window (clear all existing globs)
	:g *.mp3 - show mp3 files
:g+ - Add another glob
	:g+ *.ogg - add ogg to the list

:so - Sort Order
	:so na - name ascending
	:so nd - name descending
	:so sa - size ascending
	:so sd - size descending
	:so da - date ascending
	:so dd - date descending

:tc - Tagged Copy - Does a validated copy from the active to inactive pane
:td - Tagged Delete - Removes tagged files
:tm - Tagged Move - Does a validated copy then deletion

:tf - Tag all files using a filter
:tf - Tag all files using a glob
:ta - Tag All
:taf - Tag All Files
:tad - Tag All Directories
:tu - Untag All
:t! - Flip all tags

:sym - Symlink files or directories. Not available on WIN32 platforms

:swap - swap panels

:x - extract archive


Internal Viewer Commands
========================
:q - Quit
:t #num - Set tab size to #
:g #num - goto line number
:g string - goto bookmark
:b string - bookmark this line



Config
======
- document date format allowed
- document time format allowed...

document whole dang options file....

Windows Notes
=============
On windows, any file that starts with a dot eg ".netbeans" will have
its attribute shown as hidden, even when it does not have a hidden
attribute.


Credits
=======
Stu George - Right now, everything


Getting the Source
==================
The source is available on Gitorious

    git clone git://gitorious.org/alfc/alfc.git



Portability
===========
Notes on the MinGW port.
- MinGW requires the 'regex' package (both bin + dev) to be installed for building.
for distribution you need the regex dll from the bin package
(Put the DLL in your windows/system32 directory or somewhere like that)

- You also need a copy of the fnmatch.h header filer
http://git.savannah.gnu.org/gitweb/?p=gnulib.git;a=blob;f=lib/fnmatch.in.h;hb=HEAD
just save that as /mingw/include/fnmatch.h


Building
========
Also requires a lua 5.1+ library to link against.
And NCurses for the ncurses driver or PDCurses

To build the documentation you need Robodoc installed
(minimum 4.99.36)

The master build requires the 'rant' gem installed in Ruby.

Alternatively, there is a premake4 build system in place if you
want to build it on windows or linux and dont have the rant build system
setup.

On *nixen, you can do "premake4 gmake XXXX" where XXX is x11 or ncurses
depending on the driver,

For windows XXX is win32 or pdcurses, and you can replace gmake with vs2005 etc
(See premake4 site for more details)
http://industriousone.com/premake



Hints, Tips and Tricks
======================
- Whats the easiest way to delete all *.o files?
	:tg *.o
	:td

- What about deleting everything BUT my .c and .h files?
	:tf \.[ch]$
	:t!
	:td

- How do I refresh the current direcory?
	To refresh current directory just issue a ':c' with no parameter



ChangeLog
=========
20090111 - Started...


