/*
 * Copyright (C) 2017 <davidedmundson@kde.org> David Edmundson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "xtestsender.h"
#include <stdbool.h>
#include <xcb/xtest.h>

int xcb_test_fake_button_event(xcb_connection_t *dpy, uint8_t button, bool is_press, uint32_t delay)
{
	uint8_t type = is_press ? XCB_BUTTON_PRESS : XCB_BUTTON_RELEASE;
	xcb_test_fake_input(dpy, type, button, delay, XCB_NONE, 0, 0, 0);
	return 1;
}

void sendXTestPressed(xcb_connection_t *display, uint8_t button)
{
	xcb_test_fake_button_event(display, button, true, 0);
}

void sendXTestReleased(xcb_connection_t *display, uint8_t button)
{
	xcb_test_fake_button_event(display, button, false, 0);
}
