using GLib;
using Gtk;
using Gdk;

namespace ValaPanel
{
	[Flags]
	private enum WidthFlags
	{
		CONSTRAIN,
		ASPECT,
		FILL
	}
	public class IconGrid : Gtk.Container, Gtk.Orientable
	{
		WidthFlags fl;
		GLib.List<Gtk.Widget> children;
		int rownum;
		int colnum;
		Gdk.Window? event_window;
		private int _spacing;
		public int target_dimension
		{get; set construct;}
		public bool aspect
		{ get {return WidthFlags.ASPECT in fl;}
		  set {fl = (value == true) ?
				  fl | WidthFlags.ASPECT :
				  fl & (~WidthFlags.ASPECT);
			  this.queue_resize();}
		}
		public bool constrain
		{ get {return WidthFlags.CONSTRAIN in fl;}
		  set {fl = (value == true) ?
				  fl | WidthFlags.CONSTRAIN :
				  fl & (~WidthFlags.CONSTRAIN);
			  this.queue_resize();}
		}
		public bool fill_width
		{ get {return WidthFlags.FILL in fl;}
		  set {fl = (value == true) ?
				  fl | WidthFlags.FILL :
				  fl & (~WidthFlags.FILL);
			  this.queue_resize();}
		}
		public Gtk.Orientation orientation
		{get; set;}
		public int child_width
		{get; set construct;}
		public int spacing
		{get {return _spacing;}
		 set construct
		 {_spacing = value; this.queue_resize();}}
		public int child_height
		{get; set construct;}
		
		public IconGrid(Gtk.Orientation or, int cw, int ch, int sp,
						uint b, int td)
		{
			Object(orientation: or,
					child_width: cw,
					child_height: ch,
					spacing: sp,
					border_width: b,
					target_dimension: td
					);		
		}
		
		construct
		{
			this.set_has_window(false);
			this.set_redraw_on_allocate(false);
			this.orientation = Gtk.Orientation.HORIZONTAL;
		}
		
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
					cha.y = a.y + border;
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
			int chw = this.child_width;
			int chh = this.child_height;
			int x_delta = 0;
			if (rownum !=0 && colnum != 0 && cha.width > 0)
			{
				x_delta = (cha.width + spacing)/colnum - spacing;
				if (constrain && ((x_delta - spacing) < chw))
					chw = int.max (2,x_delta);
				var y_delta = (cha.height + spacing)/rownum - spacing;
				if ((orientation == Gtk.Orientation.HORIZONTAL) && (y_delta - spacing > chh))
					chh = int.max (2,y_delta);
			}
			var dir = this.get_direction();
			var x = (dir == Gtk.TextDirection.RTL) ? a.width - border : border;
			var y = border;
			x_delta = 0;
			var next_coord = border;
			foreach (var child in this.children)
				if (child.get_visible())
				{
					child.get_preferred_size(out req,null);
					element_check_requisition(ref req);
					cha.width = int.min(req.width,chw);
					cha.height = int.min(req.height,chh);
					if (orientation == Gtk.Orientation.HORIZONTAL)
					{
						y = next_coord;
						if (y + chh > a.height - border && y > border)
						{
							y = border;
							x = (dir == Gtk.TextDirection.RTL) ?
								x - (x_delta + spacing) : x + (x_delta + spacing);
							x_delta = 0;
							// FIXME: if fill_width and rows = 1 then allocate whole column
						}
						next_coord = y + chh + spacing;
						x_delta = int.max(cha.width,x_delta);
					}
					else
					{
						// FIXME: if fill_width then use aspect to check delta
						if(dir == Gtk.TextDirection.RTL)
						{
							next_coord = x - cha.width;
							if (x < a.width - border)
							{
								next_coord -= spacing;
								if (next_coord < border)
								{
									next_coord = a.width - border;
									y += chh + spacing;
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
								y += chh + spacing;
							}
							next_coord = x + cha.width + spacing;
						}
					}
					cha.x = x;
					if (req.height < child_height - 1)
						y += (chh - req.height) / 2;
					cha.y = y;
	
					if (!child.get_has_window())
					{
						cha.x += a.x;
						cha.y += a.y;
					}
					// FIXME: if fill_width and rows > 1 then delay allocation
					child.size_allocate(cha);
			}
		}

		public override void get_preferred_width(out int min, out int nat)
		{
			Gtk.Requisition req = Gtk.Requisition();
			int pw = this.get_allocated_width();
			this.get_size(ref req);
			pw = (pw > 1) ? pw : int.MAX;
			min = int.min(req.width,pw);
			nat = req.width;
		}
		public override void get_preferred_height(out int min, out int nat)
		{
			Gtk.Requisition req = Gtk.Requisition();
			int pw = this.get_allocated_height();
			this.get_size(ref req);
			pw = (pw > 1) ? pw : int.MAX;
			min = int.min(req.height,pw);
			nat = req.height;
		}
		private void get_size(ref Gtk.Requisition req)
		{
			var border = (int)this.get_border_width();
			var old_rows = rownum;
			var old_columns = colnum;
			Gtk.Requisition ch_req = Gtk.Requisition();
			int row = 0, w = 0;
			req.height = 0;
			req.width = 0;
			rownum = 0;
			colnum = 0;
			if (orientation == Gtk.Orientation.HORIZONTAL)
			{
/* In horizontal orientation, fit as many rows into the available height as possible.
* Then allocate as many columns as necessary.  Guard against zerodivides. */
				if (child_height + spacing != 0)
					rownum = (target_dimension + spacing - 2*border)/
												(child_height + spacing);
				if (rownum == 0)
					rownum = 1;
				foreach (var child in children)
					if (child.get_visible())
					{
						child.get_preferred_size(out ch_req, null);
						element_check_requisition(ref ch_req);
						if (row==0)
							colnum++;
						w = int.max(w,ch_req.width);
						row++;
						if (row==rownum)
						{
							row = 0;
							if (req.width>0)
								req.width+=spacing;
							req.width += w;
							row = w = 0;
						}
					}
					if (row > 0)
						req.width += w;
				/*	if ((colnum == 1) && (rownum > visible_children))
							rownum = visible_children;*/
			}
			else
			{
/* In vertical orientation, fit as many columns into the available width as possible.
 * Then allocate as many rows as necessary.  Guard against zerodivides. */
				if (child_width + spacing != 0)
					colnum = (target_dimension + spacing - 2*border)/
												(child_width + spacing);
				if (colnum == 0)
					colnum = 1;
				foreach(var child in children)
					if (child.get_visible())
					{
						child.get_preferred_size(out ch_req, null);
						element_check_requisition(ref ch_req);
						if (w > 0 && w + ch_req.width > target_dimension)
						{
							w = 0;
							rownum++;
						}
						if (w > 0)
							w += spacing;
						w +=ch_req.width;
						req.width = int.max(req.width,w);
					}
				if (w > 0)
					rownum++;
			}
		if (rownum == 0 || colnum == 0)
			req.height = 0;
		else
			req.height = (child_height + spacing) * rownum - spacing + 2*border;
		if (req.width > 0)
			req.width += 2*border;
		if (rownum != old_rows || colnum != old_columns)
			this.queue_resize();
		}
		
		public override void add (Gtk.Widget w)
		{
			children.append(w);
			w.set_parent(this);
			this.queue_resize();
		}
		public override void remove(Gtk.Widget w)
		{
			bool was_vis = w.get_visible();
			var len = children.length();
			children.remove(w);
			if (len != children.length())
			{
				w.unparent();
				if (was_vis)
					this.queue_resize();
			}
		}
		
		public int get_child_position(Gtk.Widget w)
		{
			return children.index(w);
		}
		
		public void reorder_child(Gtk.Widget child, int position)
		{
			var old_position = children.index(child);
			unowned GLib.List<Gtk.Widget>? new_link = null;
			if (position == old_position)
				return;
			if (position >= 0)
				new_link = children.nth(position);
			children.insert_before(new_link,child);
			if (child.get_visible() && this.get_visible())
				child.queue_resize();
		}
		
		public void set_geometry(Gtk.Orientation or, int chw, int chh,
								int sp, int b, int td)
		{
			this.set_border_width(b);
			orientation = or;
			child_width = chw;
			child_height = chh;
			_spacing = sp;
			target_dimension = td;
			this.queue_resize();
		}
		
		public override void realize()
		{
			Gdk.Window? window = null;
			Gtk.Allocation alloc = Gtk.Allocation();
			var border = (int)this.get_border_width();
			Gdk.WindowAttr attr = Gdk.WindowAttr();
			bool vis_win;
			this.set_realized(true);
			this.get_allocation(out alloc);
			attr.x = alloc.x + border;
			attr.y = alloc.y + border;
			attr.width = alloc.width - 2 * border;
			attr.height = alloc.height - 2 * border;
			attr.window_type = Gdk.WindowType.CHILD;
			attr.event_mask = this.get_events()|Gdk.EventMask.BUTTON_MOTION_MASK
								|Gdk.EventMask.BUTTON_PRESS_MASK
								|Gdk.EventMask.BUTTON_RELEASE_MASK
								|Gdk.EventMask.EXPOSURE_MASK
								|Gdk.EventMask.ENTER_NOTIFY_MASK
								|Gdk.EventMask.LEAVE_NOTIFY_MASK;
			vis_win = this.get_has_window();
			if (vis_win)
			{
				attr.visual = this.get_visual();
				attr.wclass = Gdk.WindowWindowClass.INPUT_OUTPUT;
				var attr_mask = Gdk.WindowAttributesType.X | Gdk.WindowAttributesType.Y | Gdk.WindowAttributesType.VISUAL;
				window = new Gdk.Window(this.get_parent_window(),attr,attr_mask);
				this.set_window(window);
				window.set_user_data(this);
			}
			else
			{
				window = (Gdk.Window)this.get_parent_window().ref_sink();
				this.set_window(window);
				attr.wclass = Gdk.WindowWindowClass.INPUT_ONLY;
				var attr_mask = Gdk.WindowAttributesType.X | Gdk.WindowAttributesType.Y;
				event_window = new Gdk.Window(window,attr, attr_mask);
				event_window.set_user_data(this);
			}
		}
		public override void unrealize()
		{
			if (event_window != null)
			{
				event_window.set_user_data(null);
				event_window.destroy();
				event_window = null;
			}
			base.unrealize();
		}
		
		public override void map()
		{
			if (event_window != null)
				event_window.show();
			base.map();
		}
		
		public override void unmap()
		{
			if (event_window != null)
				event_window.hide();
			base.unmap();
		}
		
		public override void forall_internal(bool incl, Gtk.Callback call)
		{
			foreach(var ch in children)
			{
				call(ch);
			}
		}
	}
}
