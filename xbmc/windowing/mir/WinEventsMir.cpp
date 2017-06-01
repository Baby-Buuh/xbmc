/*
 *      Copyright (C) 2016 Canonical Ltd.
 *      brandon.schaefer@canonical.com
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */


#include "WinEventsMir.h"

#include <unordered_map>
#include <mir_toolkit/mir_client_library.h>

#include "Application.h"
#include "input/Key.h"
#include "input/keyboard/XkbcommonKeymap.h"
#include "input/MouseStat.h"

namespace
{

void MirHandlePointerButton(MirPointerEvent const* pev, unsigned char state, unsigned char type)
{
  auto x = mir_pointer_event_axis_value(pev, mir_pointer_axis_x);
  auto y = mir_pointer_event_axis_value(pev, mir_pointer_axis_y);

  MirPointerButton button_state = mir_pointer_button_primary;
  static uint32_t old_button_states = 0;
  uint32_t new_button_states = mir_pointer_event_buttons(pev);
  unsigned char xbmc_button = XBMC_BUTTON_LEFT;

  button_state = MirPointerButton(new_button_states ^ old_button_states);

  switch (button_state)
  {
    case mir_pointer_button_primary:
      xbmc_button = XBMC_BUTTON_LEFT;
      break;
    case mir_pointer_button_secondary:
      xbmc_button = XBMC_BUTTON_RIGHT;
      break;
    case mir_pointer_button_tertiary:
      xbmc_button = XBMC_BUTTON_MIDDLE;
      break;
    case mir_pointer_button_forward:
      xbmc_button = XBMC_BUTTON_X1;
      break;
    case mir_pointer_button_back:
      xbmc_button = XBMC_BUTTON_X2;
       break;
    default:
      break;
  }

  old_button_states = new_button_states;

  XBMC_Event new_event;
  memset(&new_event, 0, sizeof(new_event));

  new_event.button.button = xbmc_button;
  new_event.button.state  = state;
  new_event.button.type   = type;
  new_event.button.x = x;
  new_event.button.y = y;

  CWinEvents::MessagePush(&new_event);
}

void MirHandlePointerEvent(MirPointerEvent const* pev)
{
  switch (mir_pointer_event_action(pev))
  {
    case mir_pointer_action_button_down:
      MirHandlePointerButton(pev, XBMC_PRESSED, XBMC_MOUSEBUTTONDOWN);
      break;
    case mir_pointer_action_button_up:
      MirHandlePointerButton(pev, XBMC_RELEASED, XBMC_MOUSEBUTTONUP);
      break;
    case mir_pointer_action_motion:
    {
      XBMC_Event new_event;
      memset(&new_event, 0, sizeof(new_event));

      auto x  = mir_pointer_event_axis_value(pev, mir_pointer_axis_x);
      auto y  = mir_pointer_event_axis_value(pev, mir_pointer_axis_y);
      auto dx = mir_pointer_event_axis_value(pev, mir_pointer_axis_relative_x);
      auto dy = mir_pointer_event_axis_value(pev, mir_pointer_axis_relative_y);

      new_event.type = XBMC_MOUSEMOTION;
      new_event.motion.x = x;
      new_event.motion.y = y;
      new_event.motion.xrel = dx;
      new_event.motion.yrel = dy;

      CWinEvents::MessagePush(&new_event);
      break;
    }
    default:
      break;
  }
}

XBMCMod MirModToXBMCMode(MirInputEventModifiers mir_mod)
{
  int mod = XBMCKMOD_NONE;

  if (mir_mod & mir_input_event_modifier_shift_left)
    mod |= XBMCKMOD_LSHIFT;
  if (mir_mod & mir_input_event_modifier_shift_right)
    mod |= XBMCKMOD_RSHIFT;
  if (mir_mod & mir_input_event_modifier_meta_left)
    mod |= XBMCKMOD_LSUPER;
  if (mir_mod & mir_input_event_modifier_meta_right)
    mod |= XBMCKMOD_RSUPER;
  if (mir_mod & mir_input_event_modifier_ctrl_left)
    mod |= XBMCKMOD_LCTRL;
  if (mir_mod & mir_input_event_modifier_ctrl_right)
    mod |= XBMCKMOD_RCTRL;
  if (mir_mod & mir_input_event_modifier_alt_left)
    mod |= XBMCKMOD_LALT;
  if (mir_mod & mir_input_event_modifier_alt_right)
    mod |= XBMCKMOD_RALT;
  if (mir_mod & mir_input_event_modifier_num_lock)
    mod |= XBMCKMOD_NUM;
  if (mir_mod & mir_input_event_modifier_caps_lock)
    mod |= XBMCKMOD_CAPS;

  return XBMCMod(mod);
}

void MirHandleKeyboardEvent(MirKeyboardEvent const* kev)
{
  auto action = mir_keyboard_event_action(kev);

  XBMC_Event new_event;
  memset(&new_event, 0, sizeof(new_event));

  if (action == mir_keyboard_action_down)
  {
    new_event.type = XBMC_KEYDOWN;
  }
  else
  {
    new_event.type = XBMC_KEYUP;
  }

  auto xkb_keysym = mir_keyboard_event_key_code(kev);
  auto keysym = KODI::KEYBOARD::CXkbCommon::XBMCKeyForKeysym(xkb_keysym);
  if (keysym == XBMCK_UNKNOWN)
  {
    return;
  }

  new_event.key.keysym.sym = keysym;
  new_event.key.keysym.mod = MirModToXBMCMode(mir_keyboard_event_modifiers(kev));
  new_event.key.keysym.scancode = mir_keyboard_event_scan_code(kev);

  CWinEvents::MessagePush(&new_event);
}

// Lets just handle a tap on an action up
void MirHandleTouchEvent(MirTouchEvent const* tev)
{
  XBMC_Event new_event;
  memset(&new_event, 0, sizeof(new_event));
  new_event.type = XBMC_TOUCH;

  auto pointer_count = mir_touch_event_point_count(tev);
  auto action = mir_touch_event_action(tev, 0);

  if (action == mir_touch_action_up)
  {
    // FIXME Need to test this... since reading
    // the input manager it just turns this tap into a mouse motion?
    // Does this send a up/down?
    new_event.touch.action = ACTION_TOUCH_TAP;

    new_event.touch.x = mir_touch_event_axis_value(tev, 0, mir_touch_axis_x);
    new_event.touch.y = mir_touch_event_axis_value(tev, 0, mir_touch_axis_y);

    new_event.touch.pointers = pointer_count;
    CWinEvents::MessagePush(&new_event);
  }
}

void MirHandleInput(MirInputEvent const* iev)
{
  switch (mir_input_event_get_type(iev))
  {
    case mir_input_event_type_key:
      MirHandleKeyboardEvent(mir_input_event_get_keyboard_event(iev));
      break;
    case mir_input_event_type_pointer:
      MirHandlePointerEvent(mir_input_event_get_pointer_event(iev));
      break;
    case mir_input_event_type_touch:
      MirHandleTouchEvent(mir_input_event_get_touch_event(iev));
      break;
    default:
      break;
  }
}
}

void MirHandleEvent(MirWindow* window, MirEvent const* ev, void* context)
{
  MirEventType event_type = mir_event_get_type(ev);
  switch (event_type)
  {
    case mir_event_type_input:
      MirHandleInput(mir_event_get_input_event(ev));
      break;
    default:
      break;
  }
}

bool CWinEventsMir::MessagePump()
{
  auto ret = GetQueueSize();

  while (GetQueueSize())
  {
    XBMC_Event e;
    {
      std::lock_guard<decltype(m_mutex)> event_lock(m_mutex);
      e = m_events.front();
      m_events.pop();
    }
    g_application.OnEvent(e);
  }

  return ret;
}

size_t CWinEventsMir::GetQueueSize()
{
  std::lock_guard<decltype(m_mutex)> event_lock(m_mutex);
  return m_events.size();
}

void CWinEventsMir::MessagePush(XBMC_Event* ev)
{
  std::lock_guard<decltype(m_mutex)> event_lock(m_mutex);
  m_events.push(*ev);
}
