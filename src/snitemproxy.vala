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
        public IconPixmap.from_variant(Variant pixmap_variant)
        {
            pixmap_variant.get_child(0, "i", &this.width);
            pixmap_variant.get_child(1, "i", &this.height);
            Variant bytes_variant = pixmap_variant.get_child_value(2);
            uint8[] bytes = { };
            VariantIter bytes_iterator = bytes_variant.iterator();
            uint8 byte = 0;
            while (bytes_iterator.next("y", &byte))
                bytes += byte;
            this.bytes = bytes;
        }
        public static IconPixmap[] unbox_pixmaps(Variant variant)
        {
            IconPixmap[] pixmaps = { };
            VariantIter pixmap_iterator = variant.iterator();
            Variant pixmap_variant = pixmap_iterator.next_value();
            while (pixmap_variant != null)
            {
                var pixmap = IconPixmap.from_variant(pixmap_variant);
                pixmaps += pixmap;
                pixmap_variant = pixmap_iterator.next_value();
            }
            return pixmaps;
        }
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
            this.pixmap = IconPixmap.unbox_pixmaps(variant.get_child_value(1));
            variant.get_child(2, "s", &this.title);
            variant.get_child(3, "s", &this.description);
        }
	}

    public class ManualProxy : GLib.Object {
        const string INTERFACE_NAME = "org.kde.StatusNotifierItem";

        DBusConnection conn;
        string bus_name;
        string object_path;
        uint[] signal_ids;

        public string id { get; private set; }

        public signal void new_title();
        public signal void new_icon();
        public signal void new_overlay_icon();
        public signal void new_attention_icon();
        public signal void new_tool_tip();
        public signal void new_status(string status);

        public ManualProxy(string bus_name, string object_path) throws GLib.DBusError {
            this.bus_name = bus_name;
            this.object_path = object_path;

            id = get_dbus_property("Id").get_string();

            subscribe_dbus_signal("NewTitle", new_title_callback);
            subscribe_dbus_signal("NewIcon", new_icon_callback);
            subscribe_dbus_signal("NewOverlayIcon", new_overlay_icon_callback);
            subscribe_dbus_signal("NewAttentionIcon", new_attention_icon_callback);
            subscribe_dbus_signal("NewToolTip", new_tool_tip_callback);
            subscribe_dbus_signal("NewStatus", new_status_callback);
        }

        ~ManualProxy() {
            foreach (uint id in signal_ids)
                conn.signal_unsubscribe(id);
        }

        /*
         * DBus properties
         */
        public string get_title() throws GLib.DBusError {
            return get_dbus_property("Title").get_string();
        }

        public string get_status() throws GLib.DBusError {
            return get_dbus_property("Status").get_string();
        }

        public string get_icon_name() throws GLib.DBusError {
            return get_dbus_property("IconName").get_string();
        }

        public IconPixmap[] get_icon_pixmap() throws GLib.DBusError {
            return IconPixmap.unbox_pixmaps(get_dbus_property("IconPixmap"));
        }

        public string get_overlay_icon_name() throws GLib.DBusError {
            return get_dbus_property("OverlayIconName").get_string();
        }

        public IconPixmap[] get_overlay_icon_pixmap() throws GLib.DBusError {
            return IconPixmap.unbox_pixmaps(get_dbus_property("OverlayIconPixmap"));
        }

        public string get_attention_icon_name() throws GLib.DBusError {
            return get_dbus_property("AttentionIconName").get_string();
        }

        public IconPixmap[] get_attention_icon_pixmap() throws GLib.DBusError {
            return IconPixmap.unbox_pixmaps(get_dbus_property("AttentionIconPixmap"));
        }

        public ToolTip get_tool_tip() throws GLib.DBusError {
            return ToolTip.from_variant(get_dbus_property("ToolTip"));
        }

        /*
         * Widely used, but non-standard
         */
        public string get_icon_theme_path() throws GLib.DBusError {
            return get_dbus_property("IconThemePath").get_string();
        }

        public string get_menu() throws GLib.DBusError {
            return get_dbus_property("Menu").get_string();
        }

        /*
         * Ayatana properties
         */
        public string get_ayatana_label() throws GLib.DBusError {
            return get_dbus_property("XAyatanaLabel").get_string();
        }

        public string get_ayatana_label_guide() throws GLib.DBusError {
            return get_dbus_property("XAyatanaLabelGuide").get_string();
        }

        public uint get_ayatana_ordering_index() throws GLib.DBusError {
            return get_dbus_property("XAyatanaOrderingIndex").get_uint32();
        }

        /*
         * DBus methods
         */
        public void activate(int x, int y) {
            call_dbus_method("Activate", new GLib.Variant("(ii)", x, y));
        }
        public void secondary_activate(int x, int y) {
            call_dbus_method("SecondaryActivate", new GLib.Variant("(ii)", x, y));
        }

        public void context_menu(int x, int y) {
            call_dbus_method("ContextMenu", new GLib.Variant("(ii)", x, y));
        }

        public void scroll(int delta, string orientation) {
            call_dbus_method("Scroll", new GLib.Variant("(is)", delta, orientation));
        }
        /*Ayatana Method*/
        public void x_ayatana_secondary_activate(uint32 timestamp) {
            call_dbus_method("XAyatanaSecondaryActivate", new GLib.Variant("(u)", timestamp));
        }

        /*
         * Private methods
         */
        GLib.Variant get_dbus_property(string property_name) throws GLib.DBusError {
            try {
                return conn.call_sync(
                    bus_name,
                    object_path,
                    "org.freedesktop.DBus.Properties",
                    "Get",
                    new Variant("(ss)", INTERFACE_NAME, property_name),
                    null,
                    GLib.DBusCallFlags.NONE,
                    -1,
                    null
                ).get_child_value(0).get_variant();
            } catch (GLib.DBusError error) {
                GLib.stderr.printf("get_dbus_property: %s\n", error.message);
                throw error;
            }
        }

        void subscribe_dbus_signal(string signal_name, GLib.DBusSignalCallback callback) {
            signal_ids += conn.signal_subscribe(
                bus_name,
                INTERFACE_NAME,
                signal_name,
                object_path,
                null,
                GLib.DBusSignalFlags.NONE,
                callback
            );
        }

        void new_title_callback() {
            new_title();
        }

        void new_icon_callback() {
            new_icon();
        }

        void new_overlay_icon_callback() {
            new_overlay_icon();
        }

        void new_attention_icon_callback() {
            new_attention_icon();
        }

        void new_tool_tip_callback() {
            new_tool_tip();
        }

        void new_status_callback(GLib.DBusConnection corientationonnection,
                                 string bus_name,
                                 string object_path,
                                 string interface_name,
                                 string signal_name,
                                 GLib.Variant parameters) {
            new_status(parameters.get_child_value(0).get_string());
        }

        void call_dbus_method(string method_name, GLib.Variant parameters) {
            conn.call.begin(
                bus_name,
                object_path,
                INTERFACE_NAME,
                method_name,
                parameters,
                null,
                GLib.DBusCallFlags.NONE,
                -1,
                null
            );
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
		public abstract void context_menu(int x, int y) throws Error;
		public abstract void activate(int x, int y) throws Error;
		public abstract void secondary_activate(int x, int y) throws Error;
		public abstract void scroll(int delta, string orientation) throws Error;
		public abstract void x_ayatana_secondary_activate(uint32 timestamp) throws Error;
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
