using GLib;

public class QRichTextParser : Object
{
	private enum ListType
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
	private GenericSet<string> pango_names;
	private GenericSet<string> division_names;
	private GenericSet<string> span_aliases;
	private GenericSet<string> lists;
	private GenericSet<string> newline_at_end;
	private HashTable<string,string> translated_to_pango;
	private HashTable<string,string> special_spans;
	private MarkupParseContext context;
	private string rich_markup;
	private StringBuilder pango_markup_builder;
	private ListType list_type;
	private int list_order;
	public string pango_markup
	{get; private set;}
	public Icon? icon
	{get; private set;}
	construct
	{
		pango_markup_builder = new StringBuilder();
		context = new MarkupParseContext (parser, 0, this, null);
		init_sets();
		icon = null;
	}
	public QRichTextParser (string markup)
	{
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
		newline_at_end.add("br");
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
			pango_markup_builder.append_printf("<span ");
			var i = 0;
			foreach (var attr in attr_names)
			{
				if (attr == "bgcolor")
					pango_markup_builder.append_printf("background=\"%s\" ",attr_values[i]);
				if (attr == "color")
					pango_markup_builder.append_printf("foreground=\"%s\" ",attr_values[i]);
				if (attr == "size")
					pango_markup_builder.append_printf("size=\"%s\" ",parse_size(attr_values[i]));
				if (attr == "face")
					pango_markup_builder.append_printf("face=\"%s\" ",attr_values[i]);
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
		if (name in lists)
			list_type = ListType.NONE;
	}

	private void visit_text (MarkupParseContext context, string text, size_t text_len) throws MarkupError
	{
		pango_markup_builder.append_printf("%s",text.replace("\n","").strip());
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

	public bool parse (string markup) throws MarkupError {
        return context.parse (markup, -1);
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
