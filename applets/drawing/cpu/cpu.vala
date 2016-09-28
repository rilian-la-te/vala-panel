/*
 * vala-panel
 * Copyright (C) 2015 Konstantin Pugin <ria.freelander@gmail.com>
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
using Gtk;
using Cairo;
public class CpuApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new Cpu(toplevel,settings,number);
    }
}
//TODO:Colors
public class Cpu: Applet
{
    private const uint BORDER_SIZE = 2;
    /* User, nice, system, idle */
    private struct cpu_stat
    {
        int64 u;
        int64 n;
        int64 s;
        int64 i;
    }
    private Gdk.RGBA foreground_color;
    private Cairo.Surface surface;
    private DrawingArea da;
    private uint timer;             /* Timer for periodic update */
    private double[] stats_cpu;         /* Ring buffer of CPU utilization values */
    private uint ring_cursor;           /* Cursor for ring buffer */
    private uint pixmap_width;              /* Width of drawing area pixmap; also size of ring buffer; does not include border size */
    private uint pixmap_height;         /* Height of drawing area pixmap; does not include border size */
    private cpu_stat previous_cpu_stat;     /* Previous value of cpu_stat */
    public Cpu(ValaPanel.Toplevel toplevel,
                                  GLib.Settings? settings,
                                  uint number)
    {
        base(toplevel,settings,number);
    }
    public override void create()
    {

        /* Allocate drawing area as a child of top level widget.  Enable button press events. */
        da = new DrawingArea();
        da.set_size_request(toplevel.height > 40 ? toplevel.height : 40, toplevel.height);
        da.add_events(Gdk.EventMask.BUTTON_PRESS_MASK);
        this.add(da);

        /* Clone a graphics context and set "green" as its foreground color.
         * We will use this to draw the graph. */
        foreground_color = toplevel.get_style_context().get_color(toplevel.get_state_flags());

        /* Connect signals. */
        da.configure_event.connect(configure_event_cb);
        da.draw.connect(draw_cb);
        toplevel.notify["height"].connect(()=>{
            da.set_size_request(toplevel.height > 40 ? toplevel.height : 40, toplevel.height);
        });
        /* Show the widget.  Connect a timer to refresh the statistics. */
        this.show_all();
        timer = Timeout.add(1500, cpu_update);
    }
    private void redraw_pixmap()
    {
        Context cr = new Context(surface);
        foreground_color = toplevel.get_style_context().get_color(toplevel.get_state_flags());
        cr.set_line_width (1.0);
        /* Erase pixmap. */
        cr.rectangle(0, 0, pixmap_width, pixmap_height);
        cr.set_source_rgba(0,0,0,0);
        cr.fill();

        /* Recompute pixmap. */
        uint drawing_cursor = ring_cursor;
        Gdk.cairo_set_source_rgba(cr, foreground_color);
        for (uint i = 0; i < pixmap_width; i++)
        {
            /* Draw one bar of the CPU usage graph. */
            if (stats_cpu[drawing_cursor] != 0.0)
            {
                cr.move_to(i + 0.5, pixmap_height);
                cr.line_to(i + 0.5, pixmap_height - stats_cpu[drawing_cursor] * pixmap_height);
                cr.stroke();
            }

            /* Increment and wrap drawing cursor. */
            drawing_cursor += 1;
            if (drawing_cursor >= pixmap_width)
                drawing_cursor = 0;
        }
        cr.status();
        /* Redraw pixmap. */
        da.queue_draw();
    }
    private bool cpu_update()
    {
        if (MainContext.current_source().is_destroyed())
            return false;
        if ((stats_cpu != null) && (surface != null))
        {
            /* Open statistics file and scan out CPU usage. */
            cpu_stat cpu = cpu_stat();
            Posix.FILE stat = Posix.FILE.open("/proc/stat", "r");
            if (stat == null)
                return true;
            int fscanf_result = stat.scanf("cpu %li %li %li %li", out cpu.u, out cpu.n, out cpu.s, out cpu.i);
            /* Ensure that fscanf succeeded. */
            if (fscanf_result == 4)
            {
                /* Compute delta from previous statistics. */
                cpu_stat cpu_delta = cpu_stat();
                cpu_delta.u = cpu.u - previous_cpu_stat.u;
                cpu_delta.n = cpu.n - previous_cpu_stat.n;
                cpu_delta.s = cpu.s - previous_cpu_stat.s;
                cpu_delta.i = cpu.i - previous_cpu_stat.i;

                /* Copy current to previous. */
                previous_cpu_stat = cpu;

                /* Compute user+nice+system as a fraction of total.
                 * Introduce this sample to ring buffer, increment and wrap ring buffer cursor. */
                double cpu_uns = cpu_delta.u + cpu_delta.n + cpu_delta.s;
                stats_cpu[ring_cursor] = cpu_uns / (cpu_uns + cpu_delta.i);
                ring_cursor += 1;
                if (ring_cursor >= pixmap_width)
                    ring_cursor = 0;

                /* Redraw with the new sample. */
                redraw_pixmap();
            }
        }
        return true;
    }
    protected bool configure_event_cb(Gdk.EventConfigure event)
    {
        Allocation allocation;

        this.get_allocation(out allocation);
        /* Allocate pixmap and statistics buffer without border pixels. */
        uint new_pixmap_width = (uint)int.max((int)allocation.width - (int)BORDER_SIZE * 2, 0);
        uint new_pixmap_height = (uint)int.max((int)allocation.height - (int)BORDER_SIZE * 2, 0);
        if ((new_pixmap_width > 0) && (new_pixmap_height > 0))
        {
            /* If statistics buffer does not exist or it changed size, reallocate and preserve existing data. */
            if ((stats_cpu == null) || (new_pixmap_width != pixmap_width))
            {
                double[] new_stats_cpu = new double[new_pixmap_width];
                if (stats_cpu != null)
                {
                    if (new_pixmap_width > pixmap_width)
                    {
                        /* New allocation is larger.
                         * Introduce new "oldest" samples of zero following the cursor. */
                        Posix.memcpy(&new_stats_cpu[0],
                            &stats_cpu[0], ring_cursor * sizeof(double));
                        Posix.memcpy(&new_stats_cpu[new_pixmap_width - pixmap_width + ring_cursor],
                            &stats_cpu[ring_cursor], (pixmap_width - ring_cursor) * sizeof(double));
                    }
                    else if (ring_cursor <= new_pixmap_width)
                    {
                        /* New allocation is smaller, but still larger than the ring buffer cursor.
                         * Discard the oldest samples following the cursor. */
                        Posix.memcpy(&new_stats_cpu[0],
                            &stats_cpu[0], ring_cursor * sizeof(double));
                        Posix.memcpy(&new_stats_cpu[ring_cursor],
                            &stats_cpu[pixmap_width - new_pixmap_width + ring_cursor], (new_pixmap_width - ring_cursor) * sizeof(double));
                    }
                    else
                    {
                        /* New allocation is smaller, and also smaller than the ring buffer cursor.
                         * Discard all oldest samples following the ring buffer cursor and additional samples at the beginning of the buffer. */
                        Posix.memcpy(&new_stats_cpu[0],
                            &stats_cpu[ring_cursor - new_pixmap_width], new_pixmap_width * sizeof(double));
                        ring_cursor = 0;
                    }
                }
                stats_cpu = new_stats_cpu;
            }

            /* Allocate or reallocate pixmap. */
            pixmap_width = new_pixmap_width;
            pixmap_height = new_pixmap_height;
            surface = new ImageSurface(Format.ARGB32, (int)pixmap_width, (int)pixmap_height);
            surface.status();
            /* Redraw pixmap at the new size. */
            redraw_pixmap();
        }
        return true;
    }
    public bool draw_cb(Context cr)
    {
        /* Draw the requested part of the pixmap onto the drawing area.
         * Translate it in both x and y by the border size. */
        if (surface != null)
        {
            cr.set_source_rgba(0,0,0,0);
            cr.fill();
            cr.set_source_surface(surface,
                  BORDER_SIZE, BORDER_SIZE);
            cr.paint();
            /* check_cairo_status(cr); */
        }
        return false;
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(CpuApplet));
}
