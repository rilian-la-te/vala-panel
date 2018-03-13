/*
 * xfce4-sntray-plugin
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
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
using Gtk;

namespace StatusNotifier
{
	[DBus (use_string_marshalling = true)]
	public enum Category
	{
		[DBus (value = "ApplicationStatus")]
		APPLICATION,
		[DBus (value = "Communications")]
		COMMUNICATIONS,
		[DBus (value = "SystemServices")]
		SYSTEM,
		[DBus (value = "Hardware")]
		HARDWARE,
		[DBus (value = "Other")]
		OTHER
	}

	[DBus (use_string_marshalling = true)]
	public enum Status
	{
		[DBus (value = "Passive")]
		PASSIVE,
		[DBus (value = "Active")]
		ACTIVE,
		[DBus (value = "NeedsAttention")]
		NEEDS_ATTENTION
	}
	public struct IconPixmap
	{
	    int width;
	    int height;
	    uint8[] bytes;
        public GLib.Icon? gicon()
        {
            uint[] new_bytes = (uint[]) this.bytes;
            for (int i = 0; i < new_bytes.length; i++) {
                new_bytes[i] = new_bytes[i].to_big_endian();
            }

            this.bytes = (uint8[]) new_bytes;
            for (int i = 0; i < this.bytes.length; i = i+4) {
                uint8 red = this.bytes[i];
                this.bytes[i] = this.bytes[i+2];
                this.bytes[i+2] = red;
            }
            return  new Gdk.Pixbuf.from_data(this.bytes,
                                        Gdk.Colorspace.RGB,
                                        true,
                                        8,
                                        this.width,
                                        this.height,
                                        Cairo.Format.ARGB32.stride_for_width(this.width));
        }
	}
    public struct ToolTip
	{
	    string icon_name;
	    IconPixmap[] pixmap;
	    string title;
	    string description;
        public ToolTip.from_variant(Variant variant)
        {
            variant.get_child(0, "s", &this.icon_name);
            this.pixmap = unbox_pixmaps(variant.get_child_value(1));
            variant.get_child(2, "s", &this.title);
            variant.get_child(3, "s", &this.description);
        }
        public static IconPixmap[] unbox_pixmaps(Variant variant)
        {
            IconPixmap[] pixmaps = { };
            VariantIter pixmap_iterator = variant.iterator();
            Variant pixmap_variant = pixmap_iterator.next_value();
            while (pixmap_variant != null)
            {
                var pixmap = IconPixmap();
                pixmap_variant.get_child(0, "i", &pixmap.width);
                pixmap_variant.get_child(1, "i", &pixmap.height);
                Variant bytes_variant = pixmap_variant.get_child_value(2);
                uint8[] bytes = { };
                VariantIter bytes_iterator = bytes_variant.iterator();
                uint8 byte = 0;
                while (bytes_iterator.next("y", &byte))
                    bytes += byte;
                pixmap.bytes = bytes;
                pixmaps += pixmap;
                pixmap_variant = pixmap_iterator.next_value();
            }
            return pixmaps;
        }
	}
	[DBus (name = "org.kde.StatusNotifierItem")]
	private interface ItemIface : Object
	{
		/* Base properties */
		public abstract Category category {owned get;}
		public abstract string id {owned get;}
		public abstract Status status {owned get;}
		public abstract string title {owned get;}
		public abstract int window_id {owned get;}
		/* Menu properties */
		public abstract ObjectPath menu {owned get;}
		public abstract bool items_in_menu {owned get;}
		/* MenuModel properties */
		public abstract ObjectPath x_valapanel_action_group {owned get;}
		/* Icon properties */
		public abstract string icon_theme_path {owned get;}
		public abstract string icon_name {owned get;}
		public abstract string icon_accessible_desc {owned get;}
		public abstract IconPixmap[] icon_pixmap {owned get;}
		public abstract string overlay_icon_name {owned get;}
		public abstract IconPixmap[] overlay_icon_pixmap {owned get;}
		public abstract string attention_icon_name {owned get;}
		public abstract string attention_accessible_desc {owned get;}
		public abstract IconPixmap[] attention_icon_pixmap {owned get;}
		public abstract string attention_movie_name {owned get;}
		/* Tooltip */
		public abstract ToolTip tool_tip {owned get;}
		/* Methods */
		public abstract void context_menu(int x, int y) throws IOError;
		public abstract void activate(int x, int y) throws IOError;
		public abstract void secondary_activate(int x, int y) throws IOError;
		public abstract void scroll(int delta, string orientation) throws IOError;
		public abstract void x_ayatana_secondary_activate(uint32 timestamp) throws IOError;
		/* Signals */
		public abstract signal void new_title();
		public abstract signal void new_icon();
		public abstract signal void new_icon_theme_path(string icon_theme_path);
		public abstract signal void new_attention_icon();
		public abstract signal void new_overlay_icon();
		public abstract signal void new_tool_tip();
		public abstract signal void new_status(Status status);
		/* Ayatana */
		public abstract string x_ayatana_label {owned get;}
		public abstract string x_ayatana_label_guide {owned get;}
		public abstract uint x_ayatana_ordering_index {get;}
		public abstract signal void x_ayatana_new_label(string label, string guide);
	}
}
