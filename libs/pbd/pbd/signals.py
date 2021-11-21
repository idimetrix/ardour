#!/usr/bin/python
#
#   Copyright (C) 2009-2012 Paul Davis
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

#
# This file generates the header signals_generated.h, which
# will be put in build/libs/pbd/pbd by waf.
#
# It is probably easier to read build/libs/pbd/pbd/signals_generated.h
# than this if you want to read the code!
#

from __future__ import print_function
import sys

if len(sys.argv) < 2:
    print('Syntax: %s <path>' % sys.argv[0])
    sys.exit(1)

f = open(sys.argv[1], 'w')

print("/** THIS FILE IS AUTOGENERATED by signals.py: CHANGES WILL BE LOST */\n", file=f)

# Produce a comma-separated string from a list of substrings,
# giving an optional prefix to each substring
def comma_separated(n, prefix = ""):
    r = ""
    for i in range(0, len(n)):
        if i > 0:
            r += ", "
        r += "%s%s" % (prefix, n[i])
    return r

# Generate one SignalN class definition
# @param f File to write to
# @param n Number of parameters
# @param v True to specialize the template for a void return type
def signal(f, n, v):

    # The parameters in the form A1, A2, A3, ...
    An = []
    for i in range(0, n):
        An.append("A%d" % (i + 1))

    # The parameters in the form A1 a1, A2 a2, A3 a3, ...
    Anan = []
    for a in An:
        Anan.append('%s %s' % (a, a.lower()))

    # The parameters in the form a1, a2, a3, ...
    an = []
    for a in An:
        an.append(a.lower())

    # If the template is fully specialized, use of typename SomeTypedef::iterator is illegal
    # in c++03 (should use just SomeTypedef::iterator) [although use of typename is ok in c++0x]
    # http://stackoverflow.com/questions/6076015/typename-outside-of-template
    if n == 0 and v:
        typename = ""
    else:
        typename = "typename "

    if v:
        print("/** A signal with %d parameters (specialisation for a void return) */" % n, file=f)
    else:
        print("/** A signal with %d parameters */" % n, file=f)
    if v:
        print("template <%s>" % comma_separated(An, "typename "), file=f)
        print("class Signal%d<%s> : public SignalBase" % (n, comma_separated(["void"] + An)), file=f)
    else:
        print("template <%s>" % comma_separated(["R"] + An + ["C = OptionalLastValue<R> "], "typename "), file=f)
        print("class Signal%d : public SignalBase" % n, file=f)

    print("{", file=f)
    print("public:", file=f)
    print("", file=f)
    if v:
        print("\ttypedef boost::function<void(%s)> slot_function_type;" % comma_separated(An), file=f)
        print("\ttypedef void result_type;", file=f)
    else:
        print("\ttypedef boost::function<R(%s)> slot_function_type;" % comma_separated(An), file=f)
        print("\ttypedef boost::optional<R> result_type;", file=f)

    print("", file=f)

    print("private:", file=f)

    print("""
\t/** The slots that this signal will call on emission */
\ttypedef std::map<boost::shared_ptr<Connection>, slot_function_type> Slots;
\tSlots _slots;
""", file=f)

    print("public:", file=f)
    print("", file=f)
    print("\t~Signal%d () {" % n, file=f)

    print("\t\t_in_dtor.store (true, std::memory_order_release);", file=f)
    print("\t\tGlib::Threads::Mutex::Lock lm (_mutex);", file=f)
    print("\t\t/* Tell our connection objects that we are going away, so they don't try to call us */", file=f)
    print("\t\tfor (%sSlots::const_iterator i = _slots.begin(); i != _slots.end(); ++i) {" % typename, file=f)

    print("\t\t\ti->first->signal_going_away ();", file=f)
    print("\t\t}", file=f)
    print("\t}", file=f)
    print("", file=f)

    if n == 0:
        p = ""
        q = ""
    else:
        p = ", %s" % comma_separated(Anan)
        q = ", %s" % comma_separated(an)

    print("\tstatic void compositor (%sboost::function<void(%s)> f, EventLoop* event_loop, EventLoop::InvalidationRecord* ir%s) {" % (typename, comma_separated(An), p), file=f)
    print("\t\tevent_loop->call_slot (ir, boost::bind (f%s));" % q, file=f)
    print("\t}", file=f)

    print("""
\t/** Arrange for @a slot to be executed whenever this signal is emitted.
\t  * Store the connection that represents this arrangement in @a c.
\t  *
\t  * NOTE: @a slot will be executed in the same thread that the signal is
\t  * emitted in.
\t  */

\tvoid connect_same_thread (ScopedConnection& c, const slot_function_type& slot) {
\t\tc = _connect (0, slot);
\t}

\t/** Arrange for @a slot to be executed whenever this signal is emitted.
\t  * Add the connection that represents this arrangement to @a clist.
\t  *
\t  * NOTE: @a slot will be executed in the same thread that the signal is
\t  * emitted in.
\t  */

\tvoid connect_same_thread (ScopedConnectionList& clist, const slot_function_type& slot) {
\t\tclist.add_connection (_connect (0, slot));
\t}

\t/** Arrange for @a slot to be executed in the context of @a event_loop
\t  * whenever this signal is emitted. Add the connection that represents
\t  * this arrangement to @a clist.
\t  *
\t  * If the event loop/thread in which @a slot will be executed will
\t  * outlive the lifetime of any object referenced in @a slot,
\t  * then an InvalidationRecord should be passed, allowing
\t  * any request sent to the @a event_loop and not executed
\t  * before the object is destroyed to be marked invalid.
\t  *
\t  * "outliving the lifetime" doesn't have a specific, detailed meaning,
\t  * but is best illustrated by two contrasting examples:
\t  *
\t  * 1) the main GUI event loop/thread - this will outlive more or
\t  * less all objects in the application, and thus when arranging for
\t  * @a slot to be called in that context, an invalidation record is
\t  * highly advisable.
\t  *
\t  * 2) a secondary event loop/thread which will be destroyed along
\t  * with the objects that are typically referenced by @a slot.
\t  * Assuming that the event loop is stopped before the objects are
\t  * destroyed, there is no reason to pass in an invalidation record,
\t  * and MISSING_INVALIDATOR may be used.
\t  */

\tvoid connect (ScopedConnectionList& clist,
\t              PBD::EventLoop::InvalidationRecord* ir,
\t              const slot_function_type& slot,
\t              PBD::EventLoop* event_loop) {

\t\tif (ir) {
\t\t\tir->event_loop = event_loop;
\t\t}
""", file=f)
    u = []
    for i in range(0, n):
        u.append("_%d" % (i + 1))

    if n == 0:
        p = ""
    else:
        p = ", %s" % comma_separated(u)

    print("\t\tclist.add_connection (_connect (ir, boost::bind (&compositor, slot, event_loop, ir%s)));" % p, file=f)

    print("""
\t}

\t/** See notes for the ScopedConnectionList variant of this function. This
\t *  differs in that it stores the connection to the signal in a single
\t *  ScopedConnection rather than a ScopedConnectionList.
\t */

\tvoid connect (ScopedConnection& c,
\t              PBD::EventLoop::InvalidationRecord* ir,
\t              const slot_function_type& slot,
\t              PBD::EventLoop* event_loop) {

\t\tif (ir) {
\t\t\tir->event_loop = event_loop;
\t\t}
""", file=f)
    print("\t\tc = _connect (ir, boost::bind (&compositor, slot, event_loop, ir%s));" % p, file=f)
    print("\t}", file=f)

    print("""
\t/** Emit this signal. This will cause all slots connected to it be executed
\t  * in the order that they were connected (cross-thread issues may alter
\t  * the precise execution time of cross-thread slots).
\t  */
""", file=f)

    if v:
        print("\tvoid operator() (%s)" % comma_separated(Anan), file=f)
    else:
        print("\ttypename C::result_type operator() (%s)" % comma_separated(Anan), file=f)
    print("\t{", file=f)
    print("\t\t/* First, take a copy of our list of slots as it is now */", file=f)
    print("", file=f)
    print("\t\tSlots s;", file=f)
    print("\t\t{", file=f)
    print("\t\t\tGlib::Threads::Mutex::Lock lm (_mutex);", file=f)
    print("\t\t\ts = _slots;", file=f)
    print("\t\t}", file=f)
    print("", file=f)
    if not v:
        print("\t\tstd::list<R> r;", file=f)
    print("\t\tfor (%sSlots::const_iterator i = s.begin(); i != s.end(); ++i) {" % typename, file=f)
    print("""
\t\t\t/* We may have just called a slot, and this may have resulted in
\t\t\t * disconnection of other slots from us.  The list copy means that
\t\t\t * this won't cause any problems with invalidated iterators, but we
\t\t\t * must check to see if the slot we are about to call is still on the list.
\t\t\t */
\t\t\tbool still_there = false;
\t\t\t{
\t\t\t\tGlib::Threads::Mutex::Lock lm (_mutex);
\t\t\t\tstill_there = _slots.find (i->first) != _slots.end ();
\t\t\t}

\t\t\tif (still_there) {""", file=f)
    if v:
        print("\t\t\t\t(i->second)(%s);" % comma_separated(an), file=f)
    else:
        print("\t\t\t\tr.push_back ((i->second)(%s));" % comma_separated(an), file=f)
    print("\t\t\t}", file=f)
    print("\t\t}", file=f)
    print("", file=f)
    if not v:
        print("\t\t/* Call our combiner to do whatever is required to the result values */", file=f)
        print("\t\tC c;", file=f)
        print("\t\treturn c (r.begin(), r.end());", file=f)
    print("\t}", file=f)

    print("""
\tbool empty () const {
\t\tGlib::Threads::Mutex::Lock lm (_mutex);
\t\treturn _slots.empty ();
\t}
""", file=f)
    print("""
\tbool size () const {
\t\tGlib::Threads::Mutex::Lock lm (_mutex);
\t\treturn _slots.size ();
\t}
""", file=f)

    if v:
        tp = comma_separated(["void"] + An)
    else:
        tp = comma_separated(["R"] + An + ["C"])

    print("private:", file=f)
    print("", file=f)
    print("\tfriend class Connection;", file=f)

    print("""
\tboost::shared_ptr<Connection> _connect (PBD::EventLoop::InvalidationRecord* ir, slot_function_type f)
\t{
\t\tboost::shared_ptr<Connection> c (new Connection (this, ir));
\t\tGlib::Threads::Mutex::Lock lm (_mutex);
\t\t_slots[c] = f;
#ifdef DEBUG_PBD_SIGNAL_CONNECTIONS
\t\tif (_debug_connection) {
\t\t\tstd::cerr << "+++++++ CONNECT " << this << " size now " << _slots.size() << std::endl;
\t\t\tPBD::stacktrace (std::cerr, 10);
\t\t}
#endif
\t\treturn c;
\t}""", file=f)

    print("""
\tvoid disconnect (boost::shared_ptr<Connection> c)
\t{
\t\t/* ~ScopedConnection can call this concurrently with our d'tor */
\t\tif (!_in_dtor.load (std::memory_order_acquire)) {
\t\t\tGlib::Threads::Mutex::Lock lm (_mutex);
\t\t\tif (_in_dtor.load (std::memory_order_acquire)) {
\t\t\t/* d'tor signal_going_away() took care of everything already */
\t\t\t\treturn;
\t\t\t}
\t\t\t_slots.erase (c);
\t\t\tlm.release ();

\t\t\tc->disconnected ();
#ifdef DEBUG_PBD_SIGNAL_CONNECTIONS
\t\t\tif (_debug_connection) {
\t\t\t\tstd::cerr << "------- DISCCONNECT " << this << " size now " << _slots.size() << std::endl;
\t\t\t\tPBD::stacktrace (std::cerr, 10);
\t\t\t}
\t\t}
#endif
\t}

};
""", file=f)

for i in range(0, 6):
    signal(f, i, False)
    signal(f, i, True)
