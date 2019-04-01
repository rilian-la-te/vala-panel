/*
 * xfce4-sntray-plugin
 * Copyright (C) 2019 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using GLib;

[CCode(cheader_filename="snproxy.h",cprefix="Sn",lower_case_cprefix="sn_")]
namespace Sn
{
	[CCode(cheader_filename="icon-pixmap.h")]
	public enum Category
	{
		APPLICATION,
		COMMUNICATIONS,
		SYSTEM,
		HARDWARE,
		OTHER
	}
	[CCode(cheader_filename="icon-pixmap.h")]
	public enum Status
	{
		PASSIVE,
		ACTIVE,
		ATTENTION,
	}
	public class Proxy: GLib.Object
	{
		[NoAccessorMethod]
		public string bus_name {construct;}
		[NoAccessorMethod]
		public string object_path {construct;}
		[NoAccessorMethod]
		public int icon_size {get; set construct;}
		[NoAccessorMethod]
		public bool use_symbolic {get; set construct;}
		/* Base properties */
		[NoAccessorMethod]
		public Category category {get;}
		[NoAccessorMethod]
		public string id {owned get;}
		[NoAccessorMethod]
		public string title {owned get;}
		[NoAccessorMethod]
		public Status status {get;}
		[NoAccessorMethod]
		public string accessible_desc {owned get;}
		/* Menu properties */
		[NoAccessorMethod]
		public ObjectPath menu {owned get;}
		/* Icon properties */
		[NoAccessorMethod]
		public GLib.Icon icon {owned get;}
		/* Tooltip */
		[NoAccessorMethod]
		public string tooltip_text {owned get;}
		[NoAccessorMethod]
		public GLib.Icon tooltip_icon {owned get;}
		/* Signals */
		public signal void fail();
		public signal void initialized();
		/* Ayatana */
		public string x_ayatana_label {owned get;}
		public string x_ayatana_label_guide {owned get;}
		public uint x_ayatana_ordering_index {get;}

		/*Internal Methods */
		public Proxy(string bus_name, string object_path);
		public void start();
		public void reload();
		/*DBus Methods */
		public void context_menu(int x, int y);
		public void activate(int x, int y);
		public void secondary_activate(int x, int y);
		public void scroll(int dx, int dy);
		public void ayatana_secondary_activate(uint32 timestamp) throws Error;
	}
}

