/*
 * xfce4-sntray-plugin
 * Copyright (C) 2015-2019 Konstantin Pugin <ria.freelander@gmail.com>
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

#include "rtparser.h"
#include <stdbool.h>

static void init_sets(QRichTextParser *parser);

static void visit_start(GMarkupParseContext *context, const char *element_name,
                        const char **attribute_names, const char **attribute_values, void *obj,
                        GError **error);

static void visit_end(GMarkupParseContext *context, const char *element_name, void *obj,
                      GError **error);

static void visit_text(GMarkupParseContext *context, const char *text, size_t text_len, void *obj,
                       GError **error);

static const GMarkupParser parser = { visit_start, visit_end, visit_text, NULL, NULL };

QRichTextParser *qrich_text_parser_new(const char *markup)
{
	g_return_val_if_fail(markup != NULL, NULL);
	QRichTextParser *self      = g_slice_new0(QRichTextParser);
	self->pango_markup_builder = g_string_new("");
	self->context              = g_markup_parse_context_new(&parser, 0, self, NULL);
	init_sets(self);
	self->icon        = NULL;
	self->table_depth = 0;
	self->rich_markup = g_strdup(markup);
	return self;
}

void qrich_text_parser_free(QRichTextParser *self)
{
	g_clear_pointer(&self->pango_names, g_hash_table_unref);
	g_clear_pointer(&self->division_names, g_hash_table_unref);
	g_clear_pointer(&self->span_aliases, g_hash_table_unref);
	g_clear_pointer(&self->lists, g_hash_table_unref);
	g_clear_pointer(&self->newline_at_end, g_hash_table_unref);
	g_clear_pointer(&self->translated_to_pango, g_hash_table_unref);
	g_clear_pointer(&self->special_spans, g_hash_table_unref);
	g_clear_pointer(&self->context, g_markup_parse_context_unref);
	g_clear_pointer(&self->rich_markup, g_free);
	if (self->pango_markup_builder != NULL)
		g_string_free(self->pango_markup_builder, true);
	g_clear_pointer(&self->pango_markup, g_free);
	g_clear_object(&self->icon);
	g_slice_free(QRichTextParser, self);
}

void qrich_text_parser_free(QRichTextParser *self);
static void init_sets(QRichTextParser *parser)
{
#define D(s) s, s
	parser->pango_names = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(parser->pango_names, D("i"));
	g_hash_table_insert(parser->pango_names, D("b"));
	g_hash_table_insert(parser->pango_names, D("big"));
	g_hash_table_insert(parser->pango_names, D("s"));
	g_hash_table_insert(parser->pango_names, D("small"));
	g_hash_table_insert(parser->pango_names, D("sub"));
	g_hash_table_insert(parser->pango_names, D("sup"));
	g_hash_table_insert(parser->pango_names, D("tt"));
	g_hash_table_insert(parser->pango_names, D("u"));
	parser->translated_to_pango = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(parser->translated_to_pango, "dfn", "i");
	g_hash_table_insert(parser->translated_to_pango, "cite", "i");
	g_hash_table_insert(parser->translated_to_pango, "code", "tt");
	g_hash_table_insert(parser->translated_to_pango, "em", "i");
	g_hash_table_insert(parser->translated_to_pango, "samp", "tt");
	g_hash_table_insert(parser->translated_to_pango, "strong", "b");
	g_hash_table_insert(parser->translated_to_pango, "var", "i");
	parser->division_names = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(parser->division_names, D("markup"));
	g_hash_table_insert(parser->division_names, D("div"));
	g_hash_table_insert(parser->division_names, D("dl"));
	g_hash_table_insert(parser->division_names, D("dt"));
	g_hash_table_insert(parser->division_names, D("p"));
	g_hash_table_insert(parser->division_names, D("html"));
	g_hash_table_insert(parser->division_names, D("center"));
	parser->span_aliases = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(parser->span_aliases, D("span"));
	g_hash_table_insert(parser->span_aliases, D("font"));
	g_hash_table_insert(parser->span_aliases, D("tr"));
	g_hash_table_insert(parser->span_aliases, D("td"));
	g_hash_table_insert(parser->span_aliases, D("th"));
	g_hash_table_insert(parser->span_aliases, D("table"));
	g_hash_table_insert(parser->span_aliases, D("body"));
	parser->special_spans = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(parser->special_spans, "h1", "span size=\"large\" weight=\"bold\"");
	g_hash_table_insert(parser->special_spans, "h2", "span size=\"large\" style=\"italic\"");
	g_hash_table_insert(parser->special_spans, "h3", "span size=\"large\"");
	g_hash_table_insert(parser->special_spans, "h4", "span size=\"larger\" weight=\"bold\"");
	g_hash_table_insert(parser->special_spans, "h5", "span size=\"larger\" style=\"italic\"");
	g_hash_table_insert(parser->special_spans, "h6", "span size=\"larger\"");
	parser->newline_at_end = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(parser->newline_at_end, D("hr"));
	g_hash_table_insert(parser->newline_at_end, D("tr"));
	g_hash_table_insert(parser->newline_at_end, D("li"));
	parser->lists = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(parser->lists, D("ol"));
	g_hash_table_insert(parser->lists, D("ul"));
#undef D
}

static char *string_slice(const char *self, long start, long end)
{
	char *result = NULL;
	g_return_val_if_fail(self != NULL, NULL);
	size_t string_length = strlen(self);
	bool slice_allowed   = false;
	if (start < 0)
		start = (long)string_length + start;
	if (end < 0)
		end = (long)string_length + end;
	if (start >= 0)
		slice_allowed = start <= string_length;
	else
		slice_allowed = false;
	g_return_val_if_fail(slice_allowed, NULL);
	if (end >= 0)
		slice_allowed = end <= string_length;
	else
		slice_allowed = false;
	g_return_val_if_fail(slice_allowed, NULL);
	g_return_val_if_fail(start <= end, NULL);
	result = g_strndup(((char *)self) + start, (size_t)(end - start));
	return result;
}

static long string_last_index_of(const gchar *self, const gchar *needle, gint start_index)
{
	g_return_val_if_fail(self != NULL, 0);
	g_return_val_if_fail(needle != NULL, 0);
	char *index = g_strrstr(((gchar *)self) + start_index, (gchar *)needle);
	if (index != NULL)
		return index - self;
	else
		return -1;
}

static char *string_replace(const gchar *self, const gchar *old, const gchar *replacement)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(old != NULL, NULL);
	g_return_val_if_fail(replacement != NULL, NULL);
	if (self[0] == '\0' || old[0] == '\0' || g_str_equal(old, replacement))
		return g_strdup(self);

	g_autoptr(GError) err    = NULL;
	g_autofree char *escaped = g_regex_escape_string(old, -1);
	g_autoptr(GRegex) regex  = g_regex_new(escaped, 0, 0, &err);
	if (err)
	{
		g_critical("file %s: line %d: unexpected error: %s (%s, %d)",
		           __FILE__,
		           __LINE__,
		           err->message,
		           g_quark_to_string(err->domain),
		           err->code);
		return NULL;
	}
	g_autofree char *res = g_regex_replace_literal(regex, self, -1, 0, replacement, 0, &err);
	if (err)
	{
		g_critical("file %s: line %d: unexpected error: %s (%s, %d)",
		           __FILE__,
		           __LINE__,
		           err->message,
		           g_quark_to_string(err->domain),
		           err->code);
		return NULL;
	}
	return g_strdup(res);
}

static char *parse_size(const char *size)
{
	if (strstr(size, "+"))
		return g_strdup("larger");
	else if (strstr(size, "-"))
		return "smaller";
	else if (strstr(size, "pt") || strstr(size, "px"))
		return g_strdup_printf("%d", atoi(size) * PANGO_SCALE);
	else
		return g_strdup(size);
}

// <name>
static void visit_start(GMarkupParseContext *context, const char *name,
                        const char **attribute_names, const char **attr_values, void *obj,
                        GError **error)
{
	QRichTextParser *self = (QRichTextParser *)obj;
	if (g_hash_table_lookup(self->pango_names, name))
		g_string_append_printf(self->pango_markup_builder, "<%s>", name);
	if (g_hash_table_lookup(self->translated_to_pango, name))
		g_string_append_printf(self->pango_markup_builder,
		                       "<%s>",
		                       (char *)g_hash_table_lookup(self->translated_to_pango,
		                                                   name));
	if (g_hash_table_lookup(self->division_names, name))
		g_debug("Found block. Pango markup not support blocks for now.\n");
	if (g_hash_table_lookup(self->span_aliases, name))
	{
		g_string_append_printf(self->pango_markup_builder, "<span");
		for (size_t i = 0; attribute_names[i] != NULL; i++)
		{
			const char *attr = attribute_names[i];
			if (g_str_equal(attr, "bgcolor"))
				g_string_append_printf(self->pango_markup_builder,
				                       " background=\"%s\" ",
				                       attr_values[i]);
			if (g_str_equal(attr, "color"))
				g_string_append_printf(self->pango_markup_builder,
				                       " foreground=\"%s\" ",
				                       attr_values[i]);
			if (g_str_equal(attr, "size"))
				g_string_append_printf(self->pango_markup_builder,
				                       " size=\"%s\" ",
				                       parse_size(attr_values[i]));
			if (g_str_equal(attr, "face"))
				g_string_append_printf(self->pango_markup_builder,
				                       " face=\"%s\" ",
				                       attr_values[i]);
		}
		g_string_append_printf(self->pango_markup_builder, ">");
	}
	if (g_hash_table_lookup(self->special_spans, name))
		g_string_append_printf(self->pango_markup_builder,
		                       "<%s>",
		                       (char *)g_hash_table_lookup(self->special_spans, name));
	if (g_hash_table_lookup(self->lists, name))
	{
		self->list_order = 0;
		if (g_str_equal(name, "ol"))
			self->list_type = NUM;
		else
			self->list_type = DOT;
	}
	if (g_str_equal(name, "li"))
	{
		if (self->list_type == NUM)
			g_string_append_printf(self->pango_markup_builder,
			                       "%d. ",
			                       self->list_order);
		if (self->list_type == DOT)
			g_string_append_printf(self->pango_markup_builder, "+ ");
		self->list_order++;
	}
	if (g_str_equal(name, "img"))
	{
		for (size_t i = 0; attribute_names[i] != NULL; i++)
		{
			const char *attr = attribute_names[i];
			if (g_str_equal(attr, "src") || g_str_equal(attr, "source"))
			{
				if (self->icon != NULL)
					fprintf(
					    stderr,
					    "Multiple icons is not supported. Used only first\n");
				if (attr_values[i][0] == '/')
				{
					g_autoptr(GFile) f = g_file_new_for_path(attr_values[i]);
					self->icon         = g_file_icon_new(f);
				}
				else
				{
					g_autofree char *basename =
					    g_path_get_basename(attr_values[i]);
					g_autofree char *icon_name =
					    string_slice(basename,
					                 0,
					                 string_last_index_of(basename, ".", 0));
					g_autofree char *symb_name =
					    g_strdup_printf("%s-symbolic", icon_name);
					self->icon =
					    g_themed_icon_new_with_default_fallbacks(symb_name);
				}
			}
		}
	}
	if (g_str_equal(name, "br"))
		g_string_append_printf(self->pango_markup_builder, "\n");
	if (g_str_equal(name, "table"))
		self->table_depth++;
}

// </name>
static void visit_end(GMarkupParseContext *context, const char *element_name, void *obj,
                      GError **error)
{
	QRichTextParser *self = (QRichTextParser *)obj;
	const char *ins_name;
	if (g_hash_table_contains(self->span_aliases, element_name) ||
	    g_hash_table_contains(self->special_spans, element_name))
		ins_name = "span";
	else if (g_hash_table_contains(self->translated_to_pango, element_name))
		ins_name = g_hash_table_lookup(self->translated_to_pango, element_name);
	else
		ins_name = element_name;
	if (g_hash_table_contains(self->span_aliases, element_name) ||
	    g_hash_table_contains(self->pango_names, element_name) ||
	    g_hash_table_contains(self->translated_to_pango, element_name) ||
	    g_hash_table_contains(self->special_spans, element_name))
		g_string_append_printf(self->pango_markup_builder, "</%s>", ins_name);
	if (g_hash_table_contains(self->newline_at_end, element_name))
		g_string_append_printf(self->pango_markup_builder, "\n");
	if (g_str_equal(element_name, "td"))
		g_string_append_printf(self->pango_markup_builder, " ");
	if (g_str_equal(element_name, "table"))
		self->table_depth--;
	if (g_hash_table_contains(self->lists, element_name))
		self->list_type = NONE;
}

static void visit_text(GMarkupParseContext *context, const char *text, size_t text_len, void *obj,
                       GError **error)
{
	QRichTextParser *self     = (QRichTextParser *)obj;
	g_autofree char *new_text = string_replace(text, "\n", "");
	g_autofree char *stripped = NULL;
	if (self->table_depth > 0)
		stripped = g_strstrip(string_replace(new_text, "\n", ""));
	else
		stripped = g_strdup(new_text);
	g_string_append_printf(self->pango_markup_builder, "%s", stripped);
}
static char *prepare(const char *raw)
{
	g_autofree char *str1 = NULL;
	g_autofree char *str2 = NULL;
	if (strstr(raw, "&nbsp;"))
		str1 = string_replace(raw, "&nbsp;", " ");
	if (strstr(raw, "&"))
		str2 = string_replace(str1, "&", "&amp;");
	if (str2)
		return g_strdup(str2);
	if (str1)
		return g_strdup(str2);
	return g_strdup(raw);
}

static bool parse(QRichTextParser *self, const char *markup)
{
	g_autofree char *prepared = prepare(markup);
	g_autoptr(GError) err     = NULL;
	bool res = g_markup_parse_context_parse(self->context, prepared, strlen(prepared), &err);
	if (err)
		return false;
	return res;
}

void qrich_text_parser_translate_markup(QRichTextParser *self)
{
	g_clear_object(&self->icon);
	parse(self, self->rich_markup);
	self->pango_markup = g_strdup(self->pango_markup_builder->str);
	g_string_erase(self->pango_markup_builder, 0, -1);
	if (strstr(self->pango_markup, "&"))
	{
		char *tmp = string_replace(self->pango_markup, "&", "&amp;");
		g_clear_pointer(&self->pango_markup, g_free);
		self->pango_markup = tmp;
	}
}
