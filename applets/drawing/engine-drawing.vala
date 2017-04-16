/*
 * vala-panel
 * Copyright (C) 2017 Konstantin Pugin <ria.freelander@gmail.com>
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

using ValaPanel;
public class DrawingAppletFactory : AppletEngine, Peas.ExtensionBase
{
    public string[] get_available_oafids()
    {
        return {"cpu","monitors"};
    }
    public Applet get_applet_widget_by_oafid(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string oafid,
                                    string uuid)
    {
        switch (oafid)
        {
        case "cpu":
            return new Cpu(toplevel,settings,uuid);
        case "monitors":
            return new Monitors(toplevel,settings,uuid);
        }
        assert_not_reached();
    }
}

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletEngine), typeof(DrawingAppletFactory));
}
