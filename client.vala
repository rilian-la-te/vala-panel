using GLib;

namespace DBusMenu
{
    public class Client
    {
        private HashTable<int,Item> items;
        private bool layout_update_required;
        private bool layout_update_in_progress;
        private int[] requested_props_ids;
        private uint layout_revision;
        public Iface iface {get; private set;}
        public Client(string object_name, string object_path)
        {
            items = new HashTable<int,Item>(direct_hash, direct_equal);
            layout_revision = 0;
            try{
                this.iface = Bus.get_proxy_sync(BusType.SESSION, object_name, object_path);
            } catch (Error e) {stderr.printf("Cannot get menu! Error: %s",e.message);}
            var props = new VariantDict();
            props.insert("children-display","s","submenu");
            var item = new Item(0,this,props.end(),new List<int>());
            items.insert(0,item);
            request_layout_update();
            iface.set_default_timeout(200);
            iface.layout_updated.connect((rev,parent)=>{Idle.add(request_layout_update);});
            iface.items_properties_updated.connect(props_updated_cb);
            iface.item_activation_requested.connect(request_activation_cb);
            iface.x_valapanel_item_value_changed.connect(request_value_cb);
            requested_props_ids = {};
        }
        public unowned Item? get_root_item()
        {
            return items.lookup(0);
        }
        public unowned Item? get_item(int id)
        {
            return items.lookup(id);
        }
        private void request_activation_cb(int id, uint timestamp)
        {
            get_item(id).handle_event("clicked",new Variant.int32(0),timestamp);
        }
        private void request_value_cb(int id, uint timestamp)
        {
            get_item(id).handle_event("value-changed",new Variant.double(get_item(id).get_variant_property("x-valapanel-current-value").get_double()),timestamp);
        }
        private bool request_layout_update()
        {
            if(layout_update_in_progress)
                layout_update_required = true;
            else layout_update.begin();
            return false;
        }
        /* the original implementation will only request partial layouts if somehow possible
        / we try to save us from multiple kinds of race conditions by always requesting a full layout */
        private async void layout_update()
        {
            /* Sanity check: Version can be 0 only if dbusmenu iface is not loaded yet*/
            if (iface.version < 1)
            {
                yield layout_update();
                return;
            }
            layout_update_required = false;
            layout_update_in_progress = true;
            string[2] props = {"type", "children-display"};
            uint rev;
            Variant layout;
            try{
                iface.get_layout(0,-1,props,out rev, out layout);
            } catch (Error e) {
                debug("Cannot update layout. Error: %s\n Yielding another update...\n",e.message);
                return;
            }
            parse_layout(rev,layout);
            clean_items();
            if (layout_update_required)
                yield layout_update();
            else
                layout_update_in_progress = false;
        }
        private void parse_layout(uint rev, Variant layout)
        {
            /* Removed revision handling because of bug */
    //~         if (rev < layout_revision) return;
            /* layout signature must be "(ia{sv}av)" */
            int id = layout.get_child_value(0).get_int32();
            Variant props = layout.get_child_value(1);
            Variant children = layout.get_child_value(2);
            VariantIter chiter = children.iterator();
            List<int> children_ids = new List<int>();
            for(var child = chiter.next_value(); child != null; child = chiter.next_value())
            {
                child = child.get_variant();
                parse_layout(rev,child);
                int child_id = child.get_child_value(0).get_int32();
                children_ids.append(child_id);
            }
            if (id in items)
            {
                unowned Item item = items.lookup(id);
                VariantIter props_iter = props.iterator();
                string name;
                Variant val;
                while(props_iter.next("{sv}",out name, out val))
                    item.set_variant_property(name, val);
                /* make sure our children are all at the right place, and exist */
                var old_children_ids = item.get_children_ids();
                int i = 0;
                foreach(var new_id in children_ids)
                {
                    var old_child = -1;
                    foreach(var old_id in old_children_ids)
                        if (new_id == old_id)
                        {
                            old_child = old_id;
                            old_children_ids.remove(old_id);
                            break;
                        }
                    if (old_child < 0)
                        item.add_child(new_id,i);
                    else
                        item.move_child(old_child,i);
                    i++;
                }
                foreach (var old_id in old_children_ids)
                    item.remove_child(old_id);
            }
            else
            {
                items.insert(id, new Item(id,this,props,children_ids));
                request_properties.begin(id);
            }
            layout_revision = rev;
        }
        private void clean_items()
        {
        /* Traverses the list of cached menu items and removes everyone that is not in the list
        /  so we don't keep alive unused items */
        var tag = new DateTime.now_utc();
        List<int> traverse = new List<int>();
        traverse.append(0);
        while (traverse.length() > 0) {
            var item = this.get_item(traverse.data);
            traverse.delete_link(traverse);
            item.gc_tag = tag;
            traverse.concat(item.get_children_ids());
        }
        SList<int> remover = new SList<int>();
        items.foreach((k,v)=>{if (v.gc_tag != tag) remover.append(k);});
            foreach(var i in remover)
                items.remove(i);
        }
        /* we don't need to cache and burst-send that since it will not happen that frequently */
        public void request_about_to_show(int id)
        {
            var need_update = false;
            try
            {
                iface.about_to_show(id,out need_update);
            } catch (Error e)
            {
                stderr.printf("Error requesting about to show:%s\n",e.message);
                return;
            }
            if (need_update)
                Idle.add(request_layout_update);
        }
        private async void request_properties(int id)
        {
            Variant props;
            string[] names = {};
            if (!(id in requested_props_ids))
                requested_props_ids += id;
            try{
                iface.get_group_properties(requested_props_ids,names,out props);
            } catch (GLib.Error e) {
                stderr.printf("Error requesting properties:%s\n",e.message);
                return;
            }
            requested_props_ids = {};
            parse_props(props);
        }
        private void props_updated_cb (Variant updated_props, Variant removed_props)
        {
            parse_props(updated_props);
            parse_props(removed_props);
        }
        private void parse_props(Variant props)
        {
            /*updated properties signature is a(ia{sv}), removed is a(ias)*/
            var iter = props.iterator();
            for (var props_req = iter.next_value(); props_req!=null; props_req = iter.next_value())
            {
                int req_id = props_req.get_child_value(0).get_int32();
                Variant props_id = props_req.get_child_value(1);
                var ch_iter = props_id.iterator();
                Variant? val;
                for (val = ch_iter.next_value(); val != null; val = ch_iter.next_value())
                {
                    if (val.get_type_string() == "{sv}")
                    {
                        unowned string key = val.get_child_value(0).get_string();
                        var prop = val.get_child_value(1).get_variant();
                        if (items.lookup(req_id) != null)
                            items.lookup(req_id).set_variant_property(key,prop);
                    }
                    else if (val.get_type_string() == "s")
                    {
                        unowned string key = val.get_string();
                        if (items.lookup(req_id) != null)
                            items.lookup(req_id).set_variant_property(key,null);
                    }
                }
            }
        }
        ~Client()
        {
            items.remove_all();
        }
    }
}
