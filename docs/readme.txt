Also requires a lua 5.1+ library to link against.
And NCurses for the ncurses driver or PDCurses

To build the documentation you need Robodoc installed
(minimum 4.99.36)

See help.txt for usage and information;

Copy - Verieifed copies
Delete - Deletes
Move - Does a verified copy then does a delete.


Design Spec
- Dont over-engineer
- Not supposed to supplant vi/emacs/foo as programming editor
- Is supposed to sustain normal admin tasks (file management, config file editing, etc).


Hints
- Whats the easiest way to delete all *.o files?
:tg *.o
:td

- What about everything BUT my .c and .h files?
:tf \.[ch]$
:t!
:td


ToDo List
- Recursive Copy/Delete
- Copy/Delete/Move on screen display dialogue


