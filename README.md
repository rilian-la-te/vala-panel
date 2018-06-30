Vala Panel
===

This is Vala rewrite of [SimplePanel](https://github.com/rilian-la-te/simple-panel), GTK3 LXPanel fork.

TODO
---
 * [x] Rewrite core panel in Vala (0.2)
 * [x] Rewrite builtin plugins in Vala using libpeas. (0.2)
 * [x] Write Vala Panel Plugin wrapper for LXTray from simple-panel (it is less buggy) (0.2) (done in XEmbed plugin)
 * [x] Make global menus from Unity Appindicator (rewrite it on Vala but without Ubuntu deps) (0.3) (see https://gitlab.com/vala-panel-project/vala-panel-appmenu)
 * [x] Write a window buttons applet (0.3)
 * [X] Going to C again (0.4.X)
 * [ ] Write Notification Center Applet (0.6)
 * [ ] Wayland support, make compositor and complete Wayland support(1.0)
 * [ ] Taskbar DBus library for compositor (1.0)
 * [x] ~~Redo ValaPanelIconGrid using GtkFlowBox and such wonders.~~ (not needed since I am already using FlowBox everywhere)

*TODO for 0.4.0*
 * [x] Rewrite plugins API and core into C
 * [x] Split Toplevel into 3 objects: positioner, layout and window
   * [x] Create Platform (abstraction that will handle all positioning)
   * [X] Allow Platform to be changed and independent form toplevel code
   * [x] Create X11 platform (with current positioning code)
   * [X] Introduce gravity-based positioning
   * [X] Create Layout
   * [X] Rewrite toplevel into C
   * [X] Rewrite applet into C
 * [x] Made PanelManager (done as Platform)

 It seems 0.4.0 as done, so, release it for now.

*TODO for 0.5.0*
 * [X] Drop libpeas in favor to applets-new (use GIO Extensions to implement).
 * [ ] Be prepared for GTK4
 * [ ] Replace XEmbed to proxy and move it to extras
 * [ ] Made pack-type useful

*Some notes about realization*

Plugin-based panel. Users/developers can provide their own custom applets,
which are fully integrated. They can be moved, added, removed again, and
even broken.

Dependencies:
---

*Core:*
 * GLib (>= 2.50.0)
 * GTK3 (>= 3.22.0)
 
*Plugins:*
 * libwnck (>= 3.4.7)
 * libX11
 * valac

Lastly, always set `-DCMAKE_INSTALL_PREXIX=/usr` when using cmake. Otherwise you
won't be able to start the panel on most distros.

Author
---
 * Athor <ria.freelander@gmail.com>

Special thanks
---
 * [Ikey Doherty](mailto:ikey@evolve-os.com) for icontasklist.
 * [XFCE Team](http://www.xfce.org/) for XFCE Tasklist.
 * [LXPanel team](https://git.lxde.org/gitweb/?p=lxde/lxpanel.git;a=summary) for creating a base for my fork. Vala Panel uses LXPanel's tray code and other inspirations (including generic-config-dialog.vala and most positioning code in toplevel.vala).

Inspirations
---
 * [Budgie Desktop](https://github.com/budgie-desktop/budgie-desktop)
 * [LXPanel](https://wiki.lxde.org/en/LXPanel)
