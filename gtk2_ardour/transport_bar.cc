/*
 * Copyright (C) 2005-2007 Doug McLain <doug@nostar.net>
 * Copyright (C) 2005-2017 Tim Mayberry <mojofunk@gmail.com>
 * Copyright (C) 2005-2019 Paul Davis <paul@linuxaudiosystems.com>
 * Copyright (C) 2005 Karsten Wiese <fzuuzf@googlemail.com>
 * Copyright (C) 2005 Taybin Rutkin <taybin@taybin.com>
 * Copyright (C) 2006-2015 David Robillard <d@drobilla.net>
 * Copyright (C) 2007-2012 Carl Hetherington <carl@carlh.net>
 * Copyright (C) 2008-2010 Sakari Bergen <sakari.bergen@beatwaves.net>
 * Copyright (C) 2012-2019 Robin Gareus <robin@gareus.org>
 * Copyright (C) 2013-2015 Colin Fletcher <colin.m.fletcher@googlemail.com>
 * Copyright (C) 2013-2016 John Emmas <john@creativepost.co.uk>
 * Copyright (C) 2013-2016 Nick Mainsbridge <mainsbridge@gmail.com>
 * Copyright (C) 2014-2024 Ben Loftis <ben@harrisonconsoles.com>
 * Copyright (C) 2015 André Nusser <andre.nusser@googlemail.com>
 * Copyright (C) 2016-2018 Len Ovens <len@ovenwerks.net>
 * Copyright (C) 2017 Johannes Mueller <github@johannes-mueller.org>
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

#ifdef WAF_BUILD
#include "gtk2ardour-config.h"
#include "gtk2ardour-version.h"
#endif

#include <algorithm>
#include <cmath>
#include <iostream>

#include <stdarg.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <glib.h>
#include "pbd/gstdio_compat.h"

#include <gtkmm/accelmap.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>

#include "pbd/error.h"
#include "pbd/compose.h"
#include "pbd/convert.h"
#include "pbd/failed_constructor.h"
#include "pbd/memento_command.h"
#include "pbd/openuri.h"
#include "pbd/types_convert.h"
#include "pbd/file_utils.h"
#include <pbd/localtime_r.h>
#include "pbd/pthread_utils.h"
#include "pbd/replace_all.h"
#include "pbd/scoped_file_descriptor.h"
#include "pbd/xml++.h"

#include "gtkmm2ext/application.h"
#include "gtkmm2ext/bindings.h"
#include "gtkmm2ext/gtk_ui.h"
#include "gtkmm2ext/utils.h"
#include "gtkmm2ext/window_title.h"

#include "widgets/fastmeter.h"
#include "widgets/prompter.h"
#include "widgets/tooltips.h"

#include "ardour/ardour.h"
#include "ardour/audioengine.h"
#include "ardour/meter.h"
#include "ardour/monitor_control.h"
#include "ardour/profile.h"
#include "ardour/revision.h"
#include "ardour/transport_master.h"
#include "ardour/transport_master_manager.h"
#include "ardour/triggerbox.h"
#include "ardour/track.h"
#include "ardour/vca_manager.h"
#include "ardour/utils.h"

#include "LuaBridge/LuaBridge.h"

#ifdef WINDOWS_VST_SUPPORT
#include <fst.h>
#endif
#ifdef AUDIOUNIT_SUPPORT
#include "ardour/audio_unit.h"
#endif

// fix for OSX (nsm.h has a check function, AU/Apple defines check)
#ifdef check
#undef check
#endif

#include "temporal/time.h"

#include "control_protocol/basic_ui.h"

#include "about.h"
#include "actions.h"
#include "add_route_dialog.h"
#include "ardour_message.h"
#include "ardour_ui.h"
#include "audio_clock.h"
#include "audio_region_view.h"
#include "big_clock_window.h"
#include "big_transport_window.h"
#include "bundle_manager.h"
#include "dsp_stats_window.h"
#include "duplicate_routes_dialog.h"
#include "debug.h"
#include "engine_dialog.h"
#include "export_video_dialog.h"
#include "global_port_matrix.h"
#include "gui_object.h"
#include "gui_thread.h"
#include "idleometer.h"
#include "instrument_selector.h"
#include "io_plugin_window.h"
#include "keyboard.h"
#include "keyeditor.h"
#include "library_download_dialog.h"
#include "location_ui.h"
#include "lua_script_manager.h"
#include "luawindow.h"
#include "main_clock.h"
#include "missing_plugin_dialog.h"
#include "mixer_ui.h"
#include "meterbridge.h"
#include "mouse_cursors.h"
#include "nsm.h"
#include "opts.h"
#include "plugin_dspload_window.h"
#include "plugin_manager_ui.h"
#include "processor_box.h"
#include "public_editor.h"
#include "rc_option_editor.h"
#include "recorder_ui.h"
#include "route_time_axis.h"
#include "route_params_ui.h"
#include "save_as_dialog.h"
#include "save_template_dialog.h"
#include "script_selector.h"
#include "session_archive_dialog.h"
#include "session_dialog.h"
#include "session_metadata_dialog.h"
#include "session_option_editor.h"
#include "speaker_dialog.h"
#include "splash.h"
#include "template_dialog.h"
#include "time_axis_view_item.h"
#include "time_info_box.h"
#include "timers.h"
#include "transport_masters_dialog.h"
#include "trigger_page.h"
#include "triggerbox_ui.h"
#include "utils.h"
#include "virtual_keyboard_window.h"
#include "add_video_dialog.h"
#include "plugin_selector.h"

#include "pbd/i18n.h"

#include "transport_bar.h"

using namespace ARDOUR;
using namespace ARDOUR_UI_UTILS;
using namespace PBD;
using namespace Gtkmm2ext;
using namespace ArdourWidgets;
using namespace Gtk;
using namespace std;
using namespace Editing;

static const gchar *_record_mode_strings[] = {
	N_("Layered"),
	N_("Non-Layered"),
	N_("Snd on Snd"),
	0
};

TransportBar::TransportBar ()
	: _basic_ui (0)
{
	transport_ctrl.setup (ARDOUR_UI::instance ());
	transport_ctrl.map_actions ();

	pack_start(transport_ctrl, false, false);
}

TransportBar::~TransportBar ()
{
}

void
TransportBar::set_session (Session *s)
{
	SessionHandlePtr::set_session (s);

	transport_ctrl.set_session (s);

	if (_basic_ui) {
		delete _basic_ui;
	}

	_basic_ui = new BasicUI (*s);
}