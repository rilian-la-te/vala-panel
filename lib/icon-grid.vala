using GLib;
using Gtk;
using Gdk;


namespace Panel
{
	[Flags]
	private enum WidthFlags
	{
		CONSTRAIN,
		ASPECT,
		FILL
	}
	public class IconGrid : Gtk.Container
	{
		WidthFlags fl;
		GLib.List<Gtk.Widget> children;
		int child_width;
		int child_height;
		int target_dimension;
		int rownum;
		int colnum;
		Gdk.Window event_window;
		public bool aspect
		{ get {return WidthFlags.ASPECT in fl;}
		  set {fl = (aspect == true) ?
				  fl | WidthFlags.ASPECT :
				  fl & (~WidthFlags.ASPECT);
			  this.queue_draw();}
		}
		public bool constrain
		{ get {return WidthFlags.CONSTRAIN in fl;}
		  set {fl = (constrain == true) ?
				  fl | WidthFlags.CONSTRAIN :
				  fl & (~WidthFlags.CONSTRAIN);
			  this.queue_draw();}
		}
		public bool fill
		{ get {return WidthFlags.FILL in fl;}
		  set {fl = (fill == true) ?
				  fl | WidthFlags.FILL :
				  fl & (~WidthFlags.FILL);
			  this.queue_draw();}
		}
		public Gtk.Orientation orientation
		{get; set;}
		public int spacing
		{get; set;}
		
		private void element_check_requisition(ref Gtk.Requisition req)
		{
			if ((aspect) && !(constrain) && (req.height >1) && req.width > 1)
			{
				double ratio = (double)req.width /req.height;
				req.width = this.child_height * (int)ratio;
			}
			else
				req.width = this.child_width;
			req.height = this.child_height;
		}

		public override void size_allocate(Gtk.Allocation a)
		{
			this.set_allocation(a);
			int border = (int)this.get_border_width();
			Gtk.Allocation cha = Gtk.Allocation();
			Gtk.Requisition req = Gtk.Requisition();
			if (this.get_realized())
			{
				if (!this.get_has_window())
				{
					cha.x = a.x + border;
					cha.y = a.y +border;
				}
				else
					cha.x = cha.y = 0;
				cha.width = int.max(a.width - 2 * border, 0);
				cha.height = int.max(a.height - 2 * border, 0);
			if (this.event_window != null)
				event_window.move_resize(cha.x,cha.y,
				                         cha.width,cha.height);
			if (this.get_has_window())
				this.get_window().move_resize(a.x+border,a.y+border,
				                              cha.width,cha.height);
			}
			if (this.orientation == Gtk.Orientation.HORIZONTAL && a.height > 1)
				target_dimension = a.height;
			else if (this.orientation == Gtk.Orientation.VERTICAL && a.width > 1)
				target_dimension = a.width;
			var chw = this.child_width;
			var chh = this.child_height;
			int x_delta = (cha.width + spacing)/colnum - spacing;
			int y_delta = (cha.height + spacing)/rownum - spacing;
			if (rownum >0 && colnum > 0 && cha.width > 0)
			{
					if (constrain && (x_delta < child_width))
						chw = int.max (2,x_delta);
					if ((orientation == Gtk.Orientation.HORIZONTAL) && (y_delta > child_height))
						chh = int.max (2,y_delta);
			}
			var dir = this.get_direction();
			var x = (dir == Gtk.TextDirection.RTL) ? a.width - border : border;
			var y = border;
			x_delta = 0;
			var next_coord = border;
			foreach (var child in this.children)
			{
				if (child.get_visible())
				{
					child.get_preferred_size(out req,null);
					element_check_requisition(ref req);
					cha.width = int.min(req.width,child_width);
					cha.height = int.min(req.height, child_height);
					if (orientation == Gtk.Orientation.HORIZONTAL)
					{
						y = next_coord;
						if (y + child_height > a.height - border && y > border)
						{
							y = border;
							x = (dir == Gtk.TextDirection.RTL) ?
								x - x_delta + spacing : x + x_delta + spacing;
							x_delta = 0;
							// FIXME: if fill_width and rows = 1 then allocate whole column
						}
						else
							// FIXME: if fill_width then use aspect to check delta
							if(dir == Gtk.TextDirection.RTL)
							{
								next_coord = x - cha.width;
								if (x < a.width - border)
								{
									next_coord = spacing;
									if (next_coord < border)
									{
										next_coord = a.width - border;
										y += child_height + spacing;
									}
								}
								x = next_coord;
							}
							else 
							{
								x = next_coord;
								if (x + cha.width > a.width - border && x > border)
								{
									x = border;
									y += child_height + spacing;
								}
								next_coord = x + cha.width + spacing;
							}
					}
					cha.x = x;
					if (req.height < child_height - 1)
						y += (child_height - req.height) / 2;
					cha.y = y;

					if (!child.get_has_window())
					{
						cha.x += a.x;
						cha.y += a.y;
					}
					// FIXME: if fill_width and rows > 1 then delay allocation
				}
			child.size_allocate(cha);
			}
		}

		public override void get_preferred_width(out int min, out int nat)
		{
			Gtk.Requisition req = Gtk.Requisition();
			int pw;
			this.get_preferred_size(out req, null);
			this.get_parent().get_preferred_width(out pw, null);
			min = pw;
			nat = req.width;
		}
		public override void get_preferred_height(out int min, out int nat)
		{
			Gtk.Requisition req = Gtk.Requisition();
			int pw;
			this.get_preferred_size(out req, null);
			this.get_parent().get_preferred_height(out pw, null);
			min = pw;
			nat = req.height;
		}
	}
}