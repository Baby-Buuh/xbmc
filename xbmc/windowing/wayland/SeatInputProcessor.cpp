/*
 *      Copyright (C) 2017 Team XBMC
 *      http://xbmc.org
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

#include <cassert>

#include <linux/input-event-codes.h>
#include <wayland-client-protocol.hpp>

#include "input/MouseStat.h"
#include "SeatInputProcessor.h"
#include "utils/log.h"

using namespace KODI::WINDOWING::WAYLAND;

namespace
{

/**
 * Handle change of availability of a wl_seat input capability
 * 
 * This checks whether the capability is currently available with the wl_seat
 * and whether it was bound to an instance. If there is a mismatch between
 * these two, the instance is destroyed if a capability was removed or created
 * if a capability was added.
 * 
 * \param handler CSeatInputProcessor instance
 * \param caps new capabilities
 * \param cap capability to check for
 * \param capName human-readable name of the capability for log messages
 * \param instance reference to the Wayland protocol instance that holds the
 *                 protocol corresponding to the capability
 * \param instanceProvider function that functions as factory for the Wayland
 *                         protocol instance if the capability has been added
 * \param onNewCapability function that is called after setting the new capability
 *                        instance when it was added
 */
template<typename T>
void HandleCapabilityChange(CSeatInputProcessor* handler,
                            wayland::seat_capability caps,
                            wayland::seat_capability cap,
                            std::string const & capName, T& instance,
                            std::function<T()> const & instanceProvider,
                            std::function<void()> const & onNewCapability = std::function<void()>())
{
  bool hasCapability = caps & cap;

  if (instance.proxy_has_object() != hasCapability)
  {
    // Capability changed

    if (hasCapability)
    {
      // The capability was added
      CLog::Log(LOGDEBUG, "Wayland seat %s gained capability %s", handler->GetName().c_str(), capName.c_str());
      instance = instanceProvider();
      onNewCapability();
    }
    else
    {
      // The capability was removed
      CLog::Log(LOGDEBUG, "Wayland seat %s lost capability %s", handler->GetName().c_str(), capName.c_str());
      instance.proxy_release();
    }
  }
};

int WaylandToXbmcButton(std::uint32_t button)
{
  // Wayland button is evdev code
  switch (button)
  {
    case BTN_LEFT:
      return XBMC_BUTTON_LEFT;
    case BTN_MIDDLE:
      return XBMC_BUTTON_MIDDLE;
    case BTN_RIGHT:
      return XBMC_BUTTON_RIGHT;
    default:
      return -1;
  }
}

}

CSeatInputProcessor::CSeatInputProcessor(std::uint32_t globalName, const wayland::seat_t& seat, IInputHandler* handler)
: m_globalName(globalName), m_seat(seat), m_handler(handler)
{
  assert(m_handler);

  m_seat.on_name() = [this](std::string name)
  {
    m_name = name;
  };
  m_seat.on_capabilities() = std::bind(&CSeatInputProcessor::HandleOnCapabilities, this, std::placeholders::_1);
}

void CSeatInputProcessor::HandleOnCapabilities(wayland::seat_capability caps)
{
  HandleCapabilityChange(this,
                         caps,
                         wayland::seat_capability::pointer,
                         "pointer",
                         m_pointer,
                         static_cast<std::function < wayland::pointer_t()>> (std::bind(&wayland::seat_t::get_pointer, &m_seat)),
                         std::bind(&CSeatInputProcessor::HandlePointerCapability, this));
  HandleCapabilityChange(this,
                         caps,
                         wayland::seat_capability::keyboard,
                         "keyboard",
                         m_keyboard,
                         static_cast<std::function < wayland::keyboard_t()>> (std::bind(&wayland::seat_t::get_keyboard, &m_seat)),
                         std::bind(&CSeatInputProcessor::HandleKeyboardCapability, this));
  HandleCapabilityChange(this,
                         caps,
                         wayland::seat_capability::touch,
                         "touch",
                         m_touch,
                         static_cast<std::function < wayland::touch_t()>> (std::bind(&wayland::seat_t::get_touch, &m_seat)),
                         std::bind(&CSeatInputProcessor::HandleTouchCapability, this));
}

void CSeatInputProcessor::HandlePointerCapability()
{
  m_pointer.on_enter() = [this](std::uint32_t serial, wayland::surface_t surface, std::int32_t surfaceX, std::int32_t surfaceY)
  {
    // TODO honor WinSystemBase OS cursor flag
    m_pointer.set_cursor(serial, wayland::surface_t(), 0, 0);
    m_handler->OnEnter(InputType::POINTER);
    m_pointerX = wl_fixed_to_int(surfaceX);
    m_pointerY = wl_fixed_to_int(surfaceY);
    SendMouseMotion();
  };
  m_pointer.on_leave() = [this](std::uint32_t serial, wayland::surface_t surface)
  {
    m_handler->OnLeave(InputType::POINTER);
  };
  m_pointer.on_motion() = [this](std::uint32_t time, std::int32_t surfaceX, std::int32_t surfaceY)
  {
    m_pointerX = wl_fixed_to_int(surfaceX);
    m_pointerY = wl_fixed_to_int(surfaceY);
    SendMouseMotion();
  };
  m_pointer.on_button() = [this](std::uint32_t serial, std::uint32_t time, std::uint32_t button, wayland::pointer_button_state state)
  {
    // Keep track of currently pressed buttons, we need that for motion events
    // FIXME Is the state actually used?
    int xbmcButton = WaylandToXbmcButton(button);
    if (xbmcButton < 0)
    {
      // Button is unmapped
      return;
    }

    bool pressed = (state == wayland::pointer_button_state::pressed);
    if (pressed)
    {
      m_pointerButtonState |= XBMC_BUTTON(xbmcButton);
    }
    else
    {
      m_pointerButtonState &= ~XBMC_BUTTON(xbmcButton);
    }
    SendMouseButton(xbmcButton, pressed);
  };
  m_pointer.on_axis() = [this](std::uint32_t serial, wayland::pointer_axis axis, std::int32_t value)
  {
    // For axis events we only care about the vector direction
    // and not the scalar magnitude. Every axis event callback
    // generates one scroll button event for XBMC

    // Negative is up
    unsigned char xbmcButton = (wl_fixed_to_double(value) < 0.0) ? XBMC_BUTTON_WHEELUP : XBMC_BUTTON_WHEELDOWN;
    // Simulate a single click of the wheel-equivalent "button"
    SendMouseButton(xbmcButton, true);
    SendMouseButton(xbmcButton, false);
  };

  // Wayland groups pointer events, but right now there is no benefit in
  // treating them in groups. The main use case for doing so seems to be 
  // multi-axis (i.e. diagnoal) scrolling, but we do not support this anyway.
  /*m_pointer.on_frame() = [this]()
  {
    
  };*/
}

void CSeatInputProcessor::SendMouseMotion()
{
  auto event = XBMC_Event{
    .motion =
    {
      .type = XBMC_MOUSEMOTION,
      // Could use different values for different seats, but this does not seem to be used anyway
      .which = 0,
      // Relative movement is not available without pointer confinement
      .xrel = 0, .yrel = 0,

      .state = m_pointerButtonState,
      .x = m_pointerX,
      .y = m_pointerY
    }
  };
  m_handler->OnEvent(InputType::POINTER, event);
}

void CSeatInputProcessor::SendMouseButton(unsigned char button, bool pressed)
{
  auto event = XBMC_Event{
    .button =
    {
      .type = static_cast<unsigned char> (pressed ? XBMC_MOUSEBUTTONDOWN : XBMC_MOUSEBUTTONUP),
      .button = button,
      // Could use different values for different seats, but this does not seem to be used anyway
      .which = 0,
      // FIXME How is this different from type?
      .state = static_cast<unsigned char> (pressed ? XBMC_PRESSED : XBMC_RELEASED),
      .x = m_pointerX,
      .y = m_pointerY
    }
  };
  m_handler->OnEvent(InputType::POINTER, event);
}

void CSeatInputProcessor::HandleKeyboardCapability()
{
  // TODO
}

void CSeatInputProcessor::HandleTouchCapability()
{
  // TODO
}
