/*
 * Copyright (C) 2015 Paul Davis <paul@linuxaudiosystems.com>
 * Copyright (C) 2017 Robin Gareus <robin@gareus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <gtkmm/action.h>
#include <gtkmm/frame.h>
#include <gtkmm/notebook.h>
#include <gtkmm/window.h>
#include <gtkmm/stock.h>

#include "gtkmm2ext/actions.h"
#include "gtkmm2ext/gtk_ui.h"
#include "gtkmm2ext/utils.h"
#include "gtkmm2ext/visibility_tracker.h"

#include "widgets/tabbable.h"

#include "pbd/i18n.h"

using std::string;
using namespace Gtk;
using namespace Gtkmm2ext;
using namespace ArdourWidgets;

Tabbable::Tabbable (Gtk::Widget& w, const string& visible_name, string const & nontranslatable_name, bool tabbed_by_default)
	: WindowProxy (visible_name, nontranslatable_name)
	, _contents (w)
	, _parent_notebook (0)
	, tab_requested_by_state (tabbed_by_default)
{
	_strip_attachment_button.set_text(_("Left"));
	_list_attachment_button.set_text(_("Right"));
	_prop_attachment_button.set_text(_("Btm"));

	_content_attachment_hbox.set_border_width(3);
	_content_attachment_hbox.set_spacing(3);
	_content_attachment_hbox.pack_end(_list_attachment_button, false, false);
	_content_attachment_hbox.pack_end(_prop_attachment_button, false, false);
	_content_attachment_hbox.pack_end(_strip_attachment_button, false, false);
	_content_attachments_ebox.add(_content_attachment_hbox);

	_content_header_hbox.pack_start(_content_transport_ebox, true, true);
	_content_header_hbox.pack_start(_content_attachments_ebox, false, false);
	_content_header_hbox.pack_start(_content_tabbables_ebox, false, false);

	//wrap the header eboxen in a themeable frame
	Gtk::Frame *toolbar_frame = manage(new Gtk::Frame);
	toolbar_frame->set_name ("TransportFrame");
	toolbar_frame->set_shadow_type (Gtk::SHADOW_NONE);
	toolbar_frame->add(_content_header_hbox);

	_content_vbox.pack_start(*toolbar_frame, false, false);
	_content_vbox.pack_start(_content_hbox, true, true);

	_content_hbox.pack_start(_content_strip_ebox, false, false);
	_content_hbox.pack_start(_content_midlevel_vbox, true, true);

	_content_midlevel_vbox.pack_start(_content_list_pane, true, true);
	_content_midlevel_vbox.pack_start(_content_props_ebox, false, false);

	_content_list_pane.add(_content_inner_vbox);
	_content_list_pane.add(_content_list_vbox);

	//TODO: menu switcher here?
	_content_list_vbox.pack_start(_content_list_ebox, true, true);

	_content_inner_vbox.pack_start(_content_toolbar_ebox, false, false);
	_content_inner_vbox.pack_start(_content_innermost_ebox, true, true);

	_content_list_pane.set_child_minsize (_content_list_ebox, 160); /* rough guess at width of notebook tabs */
	_content_list_pane.set_check_divider_position (true);

	_content_vbox.show_all();
}

Tabbable::~Tabbable ()
{
	if (_window) {
		delete _window;
		_window = 0;
	}
}

void
Tabbable::add_to_notebook (Notebook& notebook)
{
	_parent_notebook = &notebook;

	if (tab_requested_by_state) {
		attach ();
	}
}

Window*
Tabbable::use_own_window (bool and_pack_it)
{
	Gtk::Window* win = get (true);

	if (and_pack_it) {
		Gtk::Container* parent = _contents.get_parent();
		if (parent) {
			_contents.hide ();
			parent->remove (_contents);
		}
		_own_notebook.append_page (_contents);
		_contents.show ();
	}

	return win;

}

bool
Tabbable::window_visible () const
{
	if (!_window) {
		return false;
	}

	return _window->get_visible();
}

Window*
Tabbable::get (bool create)
{
	if (_window) {
		return _window;
	}

	if (!create) {
		return 0;
	}

	/* From here on, we're creating the window
	 */

	if ((_window = new Window (WINDOW_TOPLEVEL)) == 0) {
		return 0;
	}

	_window->add (_own_notebook);
	_own_notebook.show ();
	_own_notebook.set_show_tabs (false);

	_window->signal_map().connect (sigc::mem_fun (*this, &Tabbable::window_mapped));
	_window->signal_unmap().connect (sigc::mem_fun (*this, &Tabbable::window_unmapped));

	/* do other window-related setup */

	setup ();

	/* window should be ready for derived classes to do something with it */

	return _window;
}

void
Tabbable::show_own_window (bool and_pack_it)
{
	Gtk::Widget* parent = _contents.get_parent();
	Gtk::Allocation alloc;

	if (parent) {
		alloc = parent->get_allocation();
	}

	(void) use_own_window (and_pack_it);

	if (parent) {
		_window->set_default_size (alloc.get_width(), alloc.get_height());
	}

	tab_requested_by_state = false;

	_window->present ();
}

Gtk::Notebook*
Tabbable::tab_root_drop ()
{
	/* This is called after a drop of a tab onto the root window. Its
	 * responsibility is to return the notebook that this Tabbable's
	 * contents should be packed into before the drop handling is
	 * completed. It is not responsible for actually taking care of this
	 * packing.
	 */

	show_own_window (false);
	return &_own_notebook;
}

void
Tabbable::show_window ()
{
	make_visible ();

	if (_window && (current_toplevel() == _window)) {
		if (!_visible) { /* was hidden, update status */
			set_pos_and_size ();
		}
	}
}

/** If this Tabbable is currently parented by a tab, ensure that the tab is the
 * current one. If it is parented by a window, then toggle the visibility of
 * that window.
 */
void
Tabbable::change_visibility ()
{
	if (tabbed()) {
		_parent_notebook->set_current_page (_parent_notebook->page_num (_contents));
		return;
	}

	if (tab_requested_by_state) {
		/* should be tabbed, but currently isn't parented by a notebook */
		return;
	}

	if (_window && (current_toplevel() == _window)) {
		/* Use WindowProxy method which will rotate then hide */
		toggle();
	}
}

void
Tabbable::make_visible ()
{
	if (_window && (current_toplevel() == _window)) {
		set_pos ();
		_window->present ();
	} else {

		if (!tab_requested_by_state) {
			show_own_window (true);
		} else {
			show_tab ();
		}
	}
}

void
Tabbable::make_invisible ()
{
	if (_window && (current_toplevel() == _window)) {
		_window->hide ();
	} else {
		hide_tab ();
	}
}

void
Tabbable::detach ()
{
	show_own_window (true);
	signal_tabbed_changed (false);
}

void
Tabbable::attach ()
{
	if (!_parent_notebook) {
		return;
	}

	if (tabbed()) {
		/* already tabbed */
		return;
	}


	if (_window && current_toplevel() == _window) {
		/* unpack Tabbable from parent, put it back in the main tabbed
		 * notebook
		 */

		save_pos_and_size ();

		_contents.hide ();
		_contents.get_parent()->remove (_contents);

		/* leave the window around */

		_window->hide ();
	}

	_parent_notebook->append_page (_contents);
	_parent_notebook->set_tab_detachable (_contents);
	_parent_notebook->set_tab_reorderable (_contents);
	_parent_notebook->set_current_page (_parent_notebook->page_num (_contents));

	signal_tabbed_changed (true);

	_contents.show ();

	/* have to force this on, which is semantically correct, since
	 * the user has effectively asked for it.
	 */

	tab_requested_by_state = true;
	StateChange (*this);
}

bool
Tabbable::delete_event_handler (GdkEventAny *ev)
{
	_window->hide();

	return true;
}

bool
Tabbable::tabbed () const
{
	if (_window && (current_toplevel() == _window)) {
		return false;
	}

	if (_parent_notebook && (_parent_notebook->page_num (_contents) >= 0)) {
		return true;
	}

	return false;
}

void
Tabbable::hide_tab ()
{
	if (tabbed()) {
		_contents.hide();
		_parent_notebook->remove_page (_contents);
		StateChange (*this);
	}
}

void
Tabbable::show_tab ()
{
	if (!window_visible() && _parent_notebook) {
		if (_contents.get_parent() == 0) {
			tab_requested_by_state = true;
			add_to_notebook (*_parent_notebook);
		}
		_parent_notebook->set_current_page (_parent_notebook->page_num (_contents));
		_contents.show ();
		current_toplevel()->present ();
	}
}

Gtk::Window*
Tabbable::current_toplevel () const
{
	return dynamic_cast<Gtk::Window*> (contents().get_toplevel());
}

string
Tabbable::xml_node_name()
{
	return WindowProxy::xml_node_name();
}

bool
Tabbable::tabbed_by_default() const
{
	return tab_requested_by_state;
}

XMLNode&
Tabbable::get_state() const
{
	XMLNode& node (WindowProxy::get_state());

	node.set_property (X_("tabbed"),  tabbed());

	node.set_property (string_compose("%1%2", _menu_name, X_("-listpane-pos")).c_str(), _content_list_pane.get_divider ());

	return node;
}

int
Tabbable::set_state (const XMLNode& node, int version)
{
	int ret;

	if ((ret = WindowProxy::set_state (node, version)) != 0) {
		return ret;
	}

	if (_visible) {
		show_own_window (true);
		signal_tabbed_changed (false);
	}

	XMLNodeList children = node.children ();
	XMLNode* window_node = node.child ("Window");

	if (window_node) {
		window_node->get_property (X_("tabbed"), tab_requested_by_state);
		float fract;
		if ( window_node->get_property (string_compose("%1%2", _menu_name, X_("-listpane-pos")).c_str(), fract) ) {
			if (fract > 0.75) {
				fract = 0.75f;
			}
			_content_list_pane.set_divider (0, fract);
		}
	}


	if (!_visible) {
		if (tab_requested_by_state) {
			attach ();
		} else {
			/* this does nothing if not tabbed */
			hide_tab ();
			signal_tabbed_changed (false);
		}
	}

	return ret;
}

void
Tabbable::window_mapped ()
{
	StateChange (*this);
}

void
Tabbable::window_unmapped ()
{
	StateChange (*this);
}

void
Tabbable::showhide_sidebar_list (bool yn)
{
	if (yn) {
		_content_list_vbox.show ();
	} else {
		_content_list_vbox.hide ();
	}
}

void
Tabbable::list_button_toggled ()
{
	Glib::RefPtr<Gtk::Action> act = _list_attachment_button.get_related_action();
	if (act) {
		Glib::RefPtr<Gtk::ToggleAction> tact = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(act);
		if (tact) {
			showhide_sidebar_list (tact->get_active());
		}
	}
}

void
Tabbable::showhide_sidebar_strip (bool yn)
{
	if (yn) {
		_content_strip_ebox.show ();
	} else {
		_content_strip_ebox.hide ();
	}
}

void
Tabbable::strip_button_toggled ()
{
	Glib::RefPtr<Gtk::Action> act = _strip_attachment_button.get_related_action();
	if (act) {
		Glib::RefPtr<Gtk::ToggleAction> tact = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(act);
		if (tact) {
			showhide_sidebar_strip (tact->get_active());
		}
	}
}

void
Tabbable::showhide_btm_props (bool yn)
{
	if (yn) {
		_content_props_ebox.show ();
	} else {
		_content_props_ebox.hide ();
	}
}

void
Tabbable::props_button_toggled ()
{
	Glib::RefPtr<Gtk::Action> act = _prop_attachment_button.get_related_action();
	if (act) {
		Glib::RefPtr<Gtk::ToggleAction> tact = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(act);
		if (tact) {
			showhide_btm_props (tact->get_active());
		}
	}
}
