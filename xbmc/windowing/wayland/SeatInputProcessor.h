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
#pragma once

#include <memory>

#include <wayland-client-protocol.hpp>

#include "input/keyboard/XkbcommonKeymap.h"
#include "windowing/XBMC_events.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{

enum class InputType
{
  POINTER,
  KEYBOARD,
  TOUCH
};

class IInputHandler
{
public:
  virtual ~IInputHandler() {}
  virtual void OnEvent(InputType type, XBMC_Event& event) {}
  virtual void OnEnter(InputType type) {}
  virtual void OnLeave(InputType type) {}
};

class CSeatInputProcessor
{
public:
  CSeatInputProcessor(std::uint32_t globalName, wayland::seat_t const & seat, IInputHandler* handler);
  std::uint32_t GetGlobalName() const
  {
    return m_globalName;
  }
  std::string GetName() const
  {
    return m_name;
  }

private:
  void HandleOnCapabilities(wayland::seat_capability caps);
  void HandlePointerCapability();
  void HandleKeyboardCapability();
  void HandleTouchCapability();
  
  void SendMouseMotion();
  void SendMouseButton(unsigned char button, bool pressed);
  
  void SendKey(XBMCKey key, bool pressed);
  
  std::uint32_t m_globalName;
  wayland::seat_t m_seat;
  std::string m_name = "<unknown>";

  IInputHandler* m_handler = nullptr;
  
  wayland::pointer_t m_pointer;
  wayland::keyboard_t m_keyboard;
  wayland::touch_t m_touch;
  
  unsigned char m_pointerButtonState = 0;
  std::uint16_t m_pointerX = 0;
  std::uint16_t m_pointerY = 0;
  
  std::unique_ptr<KEYBOARD::CXkbcommonKeymap> m_keymap;
};

}
}
}