using GLib;
using Gtk;

namespace Xfce
{
	public enum TasklistGrouping
	{
		NEVER,
		ALWAYS
	}
	[CCode (cname = "XfceTasklistMClick")]
	public enum TasklistMiddleClick
	{
		NOTHING,
		CLOSE_WINDOW,
		MINIMIZE_WINDOW
	}
	[CCode (cheader_filename = "tasklist-widget.h")]
	public class Tasklist: Gtk.Container
	{
		[NoAccessorMethod]
		public bool switch_workspace_on_unminimize {get; set;}
		[NoAccessorMethod]
		public TasklistMiddleClick middle_click {get; set;}
		public void set_include_all_workspaces(bool setting);
		public void set_include_all_monitors(bool setting);
		public void set_grouping(TasklistGrouping group);
		public void set_show_labels(bool setting);
		public Tasklist();
		public void set_orientation(Gtk.Orientation orient);
		public void set_button_relief(Gtk.ReliefStyle relief);
		public void update_edge(Gtk.PositionType edge);
	}
}
