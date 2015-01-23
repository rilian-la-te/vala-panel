Vala Panel
---

Simple, yet elegant panel.

What's that? Not a fork.  Technically.

This is Vala rewrite of SimplePanel, GTK3 LXPanel fork.

*TODO:*
 * Rewrite core panel in Vala (0.2)
 * Rewrite builtin plugins in Vala using libpeas. (0.2)
 * Write VAPI for LXTray from simple-panel (it is less buggy) (0.2)
 * Make global menus from Unity Appindicator (rewrite it on Vala but without Ubuntu deps) (0.3)
 * Write Notification Center Applet (0.4)
 * Wayland support, make compositor and complete Wayland support(1.0)
 * Finish WM migration to 3.14
 * Redo menu and panel using GtkFlowBox and such wonders. (poss v10)

*Implementation note:*

All elements are written entirely from scratch, using GTK and either Vala
or C. A rewrite took place to lower the barrier of entry for new contributors
and to ease maintainence.
(Exception: Parts of the default mutter plugin currently reside in wm/legacy.*)

*vala-panel:*

Plugin based panel. Users/developers can provide their own custom applets,
which are fully integrated. They can be moved, added, removed again, and
even broken

*Dependencies:*

 * GTK3 (>= 3.12.0)
 * upower-glib (>= 0.9.20)
 * libwnck (>= 3.4.7)
 * GLib (>= 2.40.0)
 * gee-0.8 (not gee-1.0!)
 * libpeas-1.0
 * valac


Lastly, always set --prefix=/usr when using autogen.sh, or configure, otherwise you
won't be able to start the desktop on most distros

Author
===
 * Athor <ria.freelander@gmail.com>
