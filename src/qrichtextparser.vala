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

[Compact]
public class QRichTextParser
{
    internal enum ListType
    {
        NONE,
        NUM,
        DOT,
    }
    private const MarkupParser parser = {
        visit_start,
        visit_end,
        visit_text,
        null,
        null
    };
    internal GenericSet<string> pango_names;
    internal GenericSet<string> division_names;
    internal GenericSet<string> span_aliases;
    internal GenericSet<string> lists;
    internal GenericSet<string> newline_at_end;
    internal HashTable<string,string> translated_to_pango;
    internal HashTable<string,string> special_spans;
    internal MarkupParseContext context;
    internal string rich_markup;
    internal StringBuilder pango_markup_builder;
    internal ListType list_type;
    internal int list_order;
    internal int table_depth;
    public string pango_markup;
    public Icon? icon;
    public QRichTextParser (string markup)
    {
        pango_markup_builder = new StringBuilder();
        context = new MarkupParseContext (parser, 0, this, null);
        init_sets();
        icon = null;
        table_depth = 0;
        rich_markup = markup;
    }
    private void init_sets()
    {
        pango_names = new GenericSet<string>(str_hash,str_equal);
        pango_names.add("i");
        pango_names.add("b");
        pango_names.add("big");
        pango_names.add("s");
        pango_names.add("small");
        pango_names.add("sub");
        pango_names.add("sup");
        pango_names.add("tt");
        pango_names.add("u");
        translated_to_pango = new HashTable<string,string>(str_hash,str_equal);
        translated_to_pango.insert("dfn","i");
        translated_to_pango.insert("cite","i");
        translated_to_pango.insert("code","tt");
        translated_to_pango.insert("em","i");
        translated_to_pango.insert("samp","tt");
        translated_to_pango.insert("strong","b");
        translated_to_pango.insert("var","i");
        division_names = new GenericSet<string>(str_hash,str_equal);
        division_names.add("markup");
        division_names.add("div");
        division_names.add("dl");
        division_names.add("dt");
        division_names.add("p");
        division_names.add("html");
        division_names.add("center");
        span_aliases = new GenericSet<string>(str_hash,str_equal);
        span_aliases.add("span");
        span_aliases.add("font");
        span_aliases.add("tr");
        span_aliases.add("td");
        span_aliases.add("th");
        span_aliases.add("table");
        span_aliases.add("body");
        special_spans = new HashTable<string,string>(str_hash,str_equal);
        special_spans.insert("h1","span size=\"large\" weight=\"bold\"");
        special_spans.insert("h2","span size=\"large\" style=\"italic\"");
        special_spans.insert("h3","span size=\"large\"");
        special_spans.insert("h4","span size=\"larger\" weight=\"bold\"");
        special_spans.insert("h5","span size=\"larger\" style=\"italic\"");
        special_spans.insert("h6","span size=\"larger\"");
        newline_at_end = new GenericSet<string>(str_hash,str_equal);
        newline_at_end.add("hr");
        newline_at_end.add("tr");
        newline_at_end.add("li");
        lists = new GenericSet<string>(str_hash,str_equal);
        lists.add("ol");
        lists.add("ul");
    }

    // <name>
    private void visit_start (MarkupParseContext context, string name, string[] attr_names, string[] attr_values) throws MarkupError {
        if (name in pango_names)
            pango_markup_builder.append_printf("<%s>",name);
        if (name in translated_to_pango)
            pango_markup_builder.append_printf("<%s>",translated_to_pango.lookup(name));
        if (name in division_names)
            debug("Found block. Pango markup not support blocks for now.\n");
        if (name in span_aliases)
        {
            pango_markup_builder.append_printf("<span");
            var i = 0;
            foreach (var attr in attr_names)
            {
                if (attr == "bgcolor")
                    pango_markup_builder.append_printf(" background=\"%s\" ",attr_values[i]);
                if (attr == "color")
                    pango_markup_builder.append_printf(" foreground=\"%s\" ",attr_values[i]);
                if (attr == "size")
                    pango_markup_builder.append_printf(" size=\"%s\" ",parse_size(attr_values[i]));
                if (attr == "face")
                    pango_markup_builder.append_printf(" face=\"%s\" ",attr_values[i]);
                i++;
            }
            pango_markup_builder.append_printf(">");
        }
        if (name in special_spans)
            pango_markup_builder.append_printf("<%s>",special_spans.lookup(name));
        if (name in lists)
        {
            list_order = 0;
            if (name == "ol")
                list_type = ListType.NUM;
            else
                list_type = ListType.DOT;
        }
        if (name == "li")
        {
            if (list_type == ListType.NUM)
                pango_markup_builder.append_printf("%d. ",list_order);
            if (list_type == ListType.DOT)
                pango_markup_builder.append_printf("+ ");
            list_order++;
        }
        if (name == "img")
        {
            int i = 0;
            foreach(var attr in attr_names)
            {
                if (attr == "src" || attr == "source")
                {
                    if (icon != null)
                        stderr.printf("Multiple icons is not supported. Used only first\n");
                    if (attr_values[i][0] == '/')
                        icon = new FileIcon(File.new_for_path(attr_values[i]));
                    else
                    {
                        var basename = Path.get_basename(attr_values[i]);
                        var icon_name = (string)basename[0:basename.last_index_of(".")];
                        icon = new ThemedIcon.with_default_fallbacks(icon_name+"-symbolic");
                    }

                }
                i++;
            }
        }
        if (name == "br")
            pango_markup_builder.append_printf("\n");
        if (name == "table")
            table_depth++;
    }

    // </name>
    private void visit_end (MarkupParseContext context, string name) throws MarkupError
    {
        string ins_name;
        if ((name in span_aliases) || (name in special_spans))
            ins_name = "span";
        else if (name in translated_to_pango)
            ins_name = translated_to_pango.lookup(name);
        else
            ins_name = name;
        if ((name in span_aliases) || (name in pango_names) || (name in translated_to_pango) || (name in special_spans))
            pango_markup_builder.append_printf("</%s>",ins_name);
        if (name in newline_at_end)
            pango_markup_builder.append_printf("\n");
        if (name == "td")
            pango_markup_builder.append_printf(" ");
        if (name == "table")
            table_depth--;
        if (name in lists)
            list_type = ListType.NONE;
    }

    private void visit_text (MarkupParseContext context, string text, size_t text_len) throws MarkupError
    {
        string new_text = text.replace("\n","");
        if (table_depth > 0)
            new_text = text.replace("\n","").strip();
        pango_markup_builder.append_printf("%s",new_text);
    }
    private string parse_size(string size)
    {
        if (size.contains("+"))
            return "larger";
        else if (size.contains("-"))
            return "smaller";
        else if (size.contains("pt") || size.contains("px"))
            return "%d".printf(int.parse(size)*Pango.SCALE);
        else
            return size;
    }
    private string prepare (string raw)
    {
        var str = raw;
        if (str.contains("&nbsp;"))
            str = str.replace("&nbsp;"," ");
        if (str.contains("&"))
            str = str.replace("&","&amp;");
        return str;
    }
    public bool parse (string markup) throws MarkupError
    {
        return context.parse (prepare(markup), -1);
    }

    public void translate_markup()
    {
        icon = null;
        try
        {
            parse(rich_markup);
        }
        catch(Error e) {/* Not reachable, to silence a compiler*/}
        pango_markup = pango_markup_builder.str;
        pango_markup_builder.erase();
        if (pango_markup.contains("&"))
            pango_markup = pango_markup.replace("&","&amp;");
    }
}
