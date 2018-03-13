/*
 * xfce4-sntray-plugin
 * Copyright (C) 2015-2018 Konstantin Pugin <ria.freelander@gmail.com>
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
namespace StatusNotifier
{
    private Icon? find_file_icon(string? icon_name, string? path)
    {
        if (path == null || path.length == 0)
            return null;
        try
        {
            var dir = Dir.open(path);
            for (var ch = dir.read_name(); ch!= null; ch = dir.read_name())
            {
                var f = File.new_for_path(path+"/"+ch);
                if (ch[0:ch.last_index_of(".")] == icon_name)
                    return new FileIcon(f);
                var t = f.query_file_type(FileQueryInfoFlags.NONE);
                Icon? ret = null;
                if (t == FileType.DIRECTORY)
                    ret = find_file_icon(icon_name,path+"/"+ch);
                if (ret != null)
                    return ret;
            }
        } catch (Error e) {stderr.printf("%s\n",e.message);}
        return null;
    }
}
