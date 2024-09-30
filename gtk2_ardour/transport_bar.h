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

#ifndef __ardour_trans_bar_h__
#define __ardour_trans_bar_h__

#include <time.h>

/* need _BSD_SOURCE to get timersub macros */

#ifdef _BSD_SOURCE
#include <sys/time.h>
#else
#define _BSD_SOURCE
#include <sys/time.h>
#undef _BSD_SOURCE
#endif

#include <list>
#include <cmath>

#include "pbd/xml++.h"
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/fixed.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/notebook.h>
#include <gtkmm/button.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/treeview.h>
#include <gtkmm/menubar.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/adjustment.h>

#include "gtkmm2ext/gtk_ui.h"
#include "gtkmm2ext/bindings.h"
#include "gtkmm2ext/visibility_tracker.h"

#include "ardour/ardour.h"
#include "ardour/types.h"
#include "ardour/utils.h"
#include "ardour/plugin.h"
#include "ardour/session_handle.h"
#include "ardour/system_exec.h"

#include "video_timeline.h"

#include "widgets/ardour_button.h"
#include "widgets/ardour_dropdown.h"
#include "widgets/ardour_spacer.h"

#include "ardour_dialog.h"
#include "ardour_window.h"
#include "editing.h"
#include "enums.h"
#include "main_clock.h"
#include "mini_timeline.h"
#include "shuttle_control.h"
#include "startup_fsm.h"
#include "transport_control.h"
#include "transport_control_ui.h"
#include "main_clock.h"
#include "visibility_group.h"
#include "window_manager.h"

#ifdef COMPILER_MSVC
#include "about.h"
#include "add_video_dialog.h"
#include "big_clock_window.h"
#include "big_transport_window.h"
#include "bundle_manager.h"
#include "dsp_stats_window.h"
#include "engine_dialog.h"
#include "export_video_dialog.h"
#include "global_port_matrix.h"
#include "idleometer.h"
#include "io_plugin_window.h"
#include "keyeditor.h"
#include "license_manager.h"
#include "location_ui.h"
#include "lua_script_manager.h"
#include "luawindow.h"
#include "plugin_dspload_window.h"
#include "plugin_manager_ui.h"
#include "rc_option_editor.h"
#include "route_dialogs.h"
#include "route_params_ui.h"
#include "session_option_editor.h"
#include "speaker_dialog.h"
#include "transport_masters_dialog.h"
#include "virtual_keyboard_window.h"
#include "library_download_dialog.h"
#else
class About;
class AddRouteDialog;
class AddVideoDialog;
class BigClockWindow;
class BigTransportWindow;
class BundleManager;
class EngineControl;
class ExportVideoDialog;
class KeyEditor;
class LocationUIWindow;
class LuaScriptManager;
class LuaWindow;
class RCOptionEditor;
class RouteParams_UI;
class SessionOptionEditor;
class SpeakerDialog;
class GlobalPortMatrixWindow;
class IdleOMeter;
class IOPluginWindow;
class PluginDSPLoadWindow;
class PluginManagerUI;
class DspStatisticsWindow;
class TransportMastersWindow;
class VirtualKeyboardWindow;
class LibraryDownloadDialog;
#endif

class VideoTimeLine;
class ArdourKeyboard;
class AudioClock;
class ConnectionEditor;
class DuplicateRouteDialog;
class MainClock;
class Mixer_UI;
class PublicEditor;
class RecorderUI;
class TriggerPage;
class SaveAsDialog;
class SaveTemplateDialog;
class SessionDialog;
class SessionOptionEditorWindow;
class Splash;
class TimeInfoBox;
class Meterbridge;
class LicenseManager;
class LuaWindow;
class MidiTracer;
class NSM_Client;
class LevelMeterHBox;
class GUIObjectState;
class BasicUI;

namespace ARDOUR {
	class ControlProtocolInfo;
	class IO;
	class Port;
	class Route;
	class RouteGroup;
	class Location;
	class ProcessThread;
}

namespace Gtk {
	class ProgressBar;
}

namespace ArdourWidgets {
	class Prompter;
	class Tabbable;
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

	ArdourWidgets::ArdourVSpacer cuectrl_spacer;

	ArdourWidgets::ArdourButton  _cue_rec_enable;
	ArdourWidgets::ArdourButton  _cue_play_enable;

	Gtk::HBox                transport_hbox;

	MiniTimeline       mini_timeline;
	TimeInfoBox*       time_info_box;

	Gtk::Table editor_meter_table;
	ArdourWidgets::ArdourButton editor_meter_peak_display;
	LevelMeterHBox *            editor_meter;

	ArdourWidgets::ArdourVSpacer      meterbox_spacer;
	Gtk::HBox                         meterbox_spacer2;

	ArdourWidgets::ArdourVSpacer scripts_spacer;

	ArdourWidgets::ArdourButton action_script_call_btn[MAX_LUA_ACTION_BUTTONS];

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

	void cue_ffwd_state_clicked ();
	void cue_rec_state_changed ();
	void cue_rec_state_clicked ();

	void reset_peak_display ();
	void reset_route_peak_display (ARDOUR::Route*);
	void reset_group_peak_display (ARDOUR::RouteGroup*);

	bool  _clear_editor_meter;
	bool  _editor_meter_peaked;
	bool  editor_meter_peak_button_release (GdkEventButton*);

	bool bind_lua_action_script (GdkEventButton*, int);
	void action_script_changed (int i, const std::string&);

	void every_point_zero_something_seconds ();

	/* blinking alerts */
	void sync_blink (bool);
	void blink_handler (bool);
	sigc::connection blink_connection;

	PBD::ScopedConnectionList forever_connections;
	sigc::connection point_zero_something_second_connection;

};

#endif /* __trans_bar_h__ */
