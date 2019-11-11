This is a port to LV2 of Matthias Nagorni's VCF LADSPA plugins (http://www.suse.de/~mana/ladspa.html),
written by James Morris <james@jwm-art.net>

(Note from Olivier Humbert on the 23 december 2018)
James has removed these LV2 plugins from his web page (http://www.jwm-art.net/) a while ago.
I was looking for them and send a email to him, and James has been nice enough to send them
back to me. I've asked him if he would agree if I put them back online, which he agreed.
Here we are, they're back on the web for anyone who would have an interest in them.
Thank you James!
(End of the note)

Install
-------

To build:

make

To install for all users, system wide:

sudo make install-system

To install for just your user:

make install-user


NOTES
-----

This is a beta release of the LV2 VCF plugins, and as such
is subject to change in future. The LV2 URI for each
plugin is not yet stable - depending on how much interest in
this comes from the original developer, the URI may change
completely. It is suggested that before installing a future
version, you therefor un-install this version.

Some of the filters work differently in Ardour2 than in
lv2rack and ingen. I don't yet know if this is my fault or
not... They seem to work correctly in the latter two hosts
( I think ).
