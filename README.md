Vala Panel
---

This is Vala rewrite of SimplePanel, GTK3 LXPanel fork.

*TODO:*
 * [x] Rewrite core panel in Vala (0.2)
 * [x] Rewrite builtin plugins in Vala using libpeas. (0.2)
 * [x] Write Vala Panel Plugin wrapper for LXTray from simple-panel (it is less buggy) (0.2) (done in XEmbed plugin)
 * [x] Make global menus from Unity Appindicator (rewrite it on Vala but without Ubuntu deps) (0.3) (see https://github.com/rilian-la-te/vala-panel-appmenu)
 * [x] Write a window buttons applet (0.3)
 * [ ] Write Notification Center Applet (0.4)
 * [ ] Going to C again (0.4)
 * [ ] Wayland support, make compositor and complete Wayland support(1.0)
 * [ ] Taskbar DBus library for compositor (1.0)
 * [x] ~~Redo ValaPanelIconGrid using GtkFlowBox and such wonders.~~ (not needed since I am already using FlowBox everywhere)

*TODO for 0.4.0:*
 * [ ] Rewrite plugins API and core into C
 * [ ] Split Toplevel into 3 objects: positioner, layout and window
 * [ ] Made PanelManager
 * [ ] Allow positioner to be changed and dynamically linked

*Some notes about realization*

Plugin based panel. Users/developers can provide their own custom applets,
which are fully integrated. They can be moved, added, removed again, and
even broken.

*Dependencies:*

*Core:*
 * GLib (>= 2.44.0)
 * GTK3 (>= 3.16.0)
 * libpeas-1.0
 * valac
 
*Plugins:*
 * libwnck (>= 3.4.7)
 * libX11




Lastly, always set -DCMAKE_INSTALL_PREXIX=/usr when using cmake, otherwise you
won't be able to start the panel on most distros

Author
===
 * Athor <ria.freelander@gmail.com>

Special thanks:
===
 * Ikey Doherty <ikey@evolve-os.com> for sidebar widget and inspirations.
 * LXPanel team <https://github.com/lxde/lxpanel> for doing a base for my fork.
 Altrough one piece from lxpanel code in this is tray, but many inspirations is carrying around
 (Notable examples is generic-config-dialog.vala and most positioning and autohide code in toplevel.vala)

Inspirations
===
 * Budgie Desktop <https://github.com/evolve-os/budgie-desktop/>
 * LXPanel <https://github.com/lxde/lxpanel>
