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

#ifndef _WIDGETS_TABBABLE_H_
#define _WIDGETS_TABBABLE_H_

#include <gtkmm/bin.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>

#include "gtkmm2ext/window_proxy.h"

#include "widgets/ardour_button.h"
#include "widgets/eventboxext.h"
#include "widgets/pane.h"
#include "widgets/visibility.h"

namespace Gtk {
	class Window;
	class Notebook;
}

namespace Gtkmm2ext {
	class VisibilityTracker;
}

namespace ArdourWidgets {

class LIBWIDGETS_API Tabbable : public Gtkmm2ext::WindowProxy
{
public:
	Tabbable (Gtk::Widget&, const std::string& user_visible_name, std::string const & untranslated_name, bool tabbed_by_default = true);
	~Tabbable ();

	void add_to_notebook (Gtk::Notebook& notebook);
	void make_visible ();
	void make_invisible ();
	void change_visibility ();
	void attach ();
	void detach ();

	Gtk::Widget& contents() const { return _contents; }

	Gtk::Window* get (bool create = false);
	Gtk::Window* own_window () { return get (false); }
	virtual Gtk::Window* use_own_window (bool and_pack_it);

	void set_default_tabbed (bool yn);

	virtual void show_window ();

	bool window_visible () const;
	bool tabbed() const;
	bool tabbed_by_default () const;


	Gtk::Window* current_toplevel () const;

	Gtk::Notebook* tab_root_drop ();

	int set_state (const XMLNode&, int version);
	XMLNode& get_state () const;

	static std::string xml_node_name();

	sigc::signal1<void,Tabbable&> StateChange;

	/* this is where ArdourUI packs the tab switchers
	* (record/cues/edit/mix) into my toolbar area,
	* in the case where I'm attached to the main window
	*/
	Gtk::EventBox&  content_tabbables_ebox() {return _content_tabbables_ebox;}

	void strip_button_toggled();
	void list_button_toggled();
	void props_button_toggled();

protected:
	bool delete_event_handler (GdkEventAny *ev);

	sigc::signal1<void, bool> signal_tabbed_changed;

	/* This is the heirarchy of a Tabbable's widget packing.
	 * The end result is to provide 8(ish) event-boxen where the tab can put its contents
	 * Please maintain the indention here so the hierarchy is visible
	*/

	/* clang-format off */
	Gtk::VBox     _content_vbox;                     /* this is the root widget for a full-featured tabbable, which contains:  */
	/*              toolbar_frame                     * the frame is managed in the implementation */
	Gtk::HBox       _content_header_hbox;
	EventBoxExt       _content_transport_ebox;       /* a placeholder for the transport bar, if you want one */
	Gtk::EventBox     _content_attachments_ebox;     /* a placeholder the (strip, list, props) visibility buttons for this tab */
	Gtk::HBox           _content_attachment_hbox;
	EventBoxExt       _content_tabbables_ebox;       /* a placeholder for the tabbable switching buttons (used by ArdourUI) */
	Gtk::HBox       _content_hbox;
	EventBoxExt       _content_strip_ebox;           /* a placeholder for the mixer strip, if you want one */
	Gtk::VBox         _content_midlevel_vbox;
	HPane               _content_list_pane;
	Gtk::VBox             _content_inner_vbox;
	EventBoxExt             _content_toolbar_ebox;   /* a placeholder for the content-specific toolbar, if you want one */
	Gtk::HBox               _content_innermost_ebox; /* a placeholder for the innermost content (recorder, cues, editor, mixer) */
	Gtk::VBox             _content_list_vbox;
	EventBoxExt           _content_list_ebox;        /* a placeholder for the sidebar list, if you want one */
	EventBoxExt         _content_props_ebox;         /* a placeholder for the property box, if you want one */
	/* clang-format on */

	Gtk::Widget&   _contents;  //for most Tabbables this will be _content_vbox;  but rc_options, for example, does something different.

	/* visibility controls */
	ArdourWidgets::ArdourButton _strip_attachment_button;
	ArdourWidgets::ArdourButton _list_attachment_button;
	ArdourWidgets::ArdourButton _prop_attachment_button;

	void showhide_sidebar_strip (bool yn);
	void showhide_sidebar_list (bool yn);
	void showhide_btm_props (bool yn);

private:

	Gtk::Notebook  _own_notebook;
	Gtk::Notebook* _parent_notebook;
	bool            tab_requested_by_state;

	void show_tab ();
	void hide_tab ();
	bool tab_close_clicked (GdkEventButton*);
	void show_own_window (bool and_pack_it);
	void window_mapped ();
	void window_unmapped ();
};

} /* end namespace */

#endif
