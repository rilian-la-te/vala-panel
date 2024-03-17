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
 * [x] Going to C again (0.4.X)
 * [ ] Write Notification Center Applet (0.6)
 * [ ] Wayland support, make compositor and complete Wayland support(1.0)
 * [ ] Taskbar DBus library for compositor (1.0)
 * [ ] Be prepared for GTK4
 * [x] ~~Redo ValaPanelIconGrid using GtkFlowBox and such wonders.~~ (not needed since I am already using FlowBox everywhere)

*TODO for next version*
 * [x] Remove XEmbed - it is not useful for Wayland
 * [ ] Write tasklist and some management applets for Wayland (wlroots)
 * [x] Implement Layer Shell Wayland backend

*Some notes about realization*

Plugin-based panel. Users/developers can provide their own custom applets,
which are fully integrated. They can be moved, added, removed again, and
even broken.

Dependencies:
---

*Core:*
 * GLib (>= 2.56.0)
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
 * [LXPanel team](https://git.lxde.org/gitweb/?p=lxde/lxpanel.git;a=summary) for creating a base for my fork. Vala Panel uses LXPanel's tray code and other inspirations (generic-config-dialog for applets and lxpanel-like remote command system).

Inspirations
---
 * [Budgie Desktop](https://github.com/budgie-desktop/budgie-desktop)
 * [LXPanel](https://wiki.lxde.org/en/LXPanel)
