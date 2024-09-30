/*
 * Copyright (C) 2005-2024 Paul Davis <paul@linuxaudiosystems.com>
 * Copyright (C) 2005 Karsten Wiese <fzuuzf@googlemail.com>
 * Copyright (C) 2005 Taybin Rutkin <taybin@taybin.com>
 * Copyright (C) 2006-2007 Doug McLain <doug@nostar.net>
 * Copyright (C) 2006-2011 David Robillard <d@drobilla.net>
 * Copyright (C) 2007-2012 Carl Hetherington <carl@carlh.net>
 * Copyright (C) 2007-2015 Tim Mayberry <mojofunk@gmail.com>
 * Copyright (C) 2008 Hans Baier <hansfbaier@googlemail.com>
 * Copyright (C) 2012-2015 Colin Fletcher <colin.m.fletcher@googlemail.com>
 * Copyright (C) 2013-2015 Nick Mainsbridge <mainsbridge@gmail.com>
 * Copyright (C) 2013-2016 John Emmas <john@creativepost.co.uk>
 * Copyright (C) 2013-2019 Robin Gareus <robin@gareus.org>
 * Copyright (C) 2014-2024 Ben Loftis <ben@harrisonconsoles.com>
 * Copyright (C) 2017 Johannes Mueller <github@johannes-mueller.org>
 * Copyright (C) 2018 Len Ovens <len@ovenwerks.net>
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

#pragma once

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>

#include "ardour/ardour.h"
#include "ardour/types.h"
#include "ardour/session_handle.h"

#include "widgets/ardour_button.h"
#include "widgets/ardour_dropdown.h"
#include "widgets/ardour_spacer.h"

#include "main_clock.h"
#include "mini_timeline.h"
#include "shuttle_control.h"
#include "startup_fsm.h"
#include "transport_control.h"
#include "transport_control_ui.h"
#include "main_clock.h"
#include "visibility_group.h"
#include "window_manager.h"

class BasicUI;
class TimeInfoBox;
class LevelMeterHBox;

namespace ARDOUR {
	class Route;
	class RouteGroup;
}

class TransportBar : public Gtk::HBox, public ARDOUR::SessionHandlePtr
{
public:
	TransportBar ();
	~TransportBar();

	void set_session (ARDOUR::Session *);

	void focus_on_clock ();

private:
	Gtk::Table    transport_table;

	BasicUI*      _basic_ui;

	TransportControlUI transport_ctrl;

	ShuttleControl     shuttle_box;

	ArdourWidgets::ArdourButton sync_button;

	Gtk::Label   punch_space;

	ArdourWidgets::ArdourVSpacer recpunch_spacer;

	Gtk::Label   punch_label;
	ArdourWidgets::ArdourButton   punch_in_button;
	ArdourWidgets::ArdourButton   punch_out_button;

	Gtk::Label   layered_label;
	ArdourWidgets::ArdourDropdown record_mode_selector;
	std::vector<std::string> record_mode_strings;

	ArdourWidgets::ArdourVSpacer latency_spacer;

	ArdourWidgets::ArdourButton latency_disable_button;

	Gtk::Label route_latency_value;
	Gtk::Label io_latency_label;
	Gtk::Label io_latency_value;

	ArdourWidgets::ArdourButton follow_edits_button;
	ArdourWidgets::ArdourButton auto_return_button;

	TransportClock primary_clock;
	TransportClock secondary_clock;

	ArdourWidgets::ArdourVSpacer* secondary_clock_spacer;

	ArdourWidgets::ArdourButton auditioning_alert_button;
	ArdourWidgets::ArdourButton solo_alert_button;
	ArdourWidgets::ArdourButton feedback_alert_button;

	Gtk::VBox alert_box;

	ArdourWidgets::ArdourVSpacer monitor_spacer;

	ArdourWidgets::ArdourButton monitor_dim_button;
	ArdourWidgets::ArdourButton monitor_mono_button;
	ArdourWidgets::ArdourButton monitor_mute_button;

	//button actions
	bool sync_button_clicked (GdkEventButton *);

	void set_record_mode (ARDOUR::RecordMode);

	void parameter_changed (std::string);

	void repack_transport_hbox ();

	void map_transport_state ();

	void set_transport_sensitivity (bool);

	void latency_switch_changed ();
	void session_latency_updated (bool);

	void update_clock_visibility ();

	void solo_blink (bool);
	void audition_blink (bool);
	void feedback_blink (bool);

	void soloing_changed (bool);
	void auditioning_changed (bool);
	void _auditioning_changed (bool);

	void feedback_detected ();
	void successful_graph_sort ();
	bool _feedback_exists;
	bool _ambiguous_latency;

	bool solo_alert_press (GdkEventButton* ev);
	void audition_alert_clicked ();

	/* blinking alerts */
	void sync_blink (bool);
	void blink_handler (bool);
	sigc::connection blink_connection;

	PBD::ScopedConnectionList forever_connections;
};
