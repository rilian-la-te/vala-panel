using GLib;
using Gtk;

namespace Xfce
{
	[CCode (cheader_filename = "tasklist-widget.h")]
	public class Tasklist: Gtk.Container
	{
		public Tasklist();
		public void set_orientation(Gtk.Orientation orient);
	}
}
