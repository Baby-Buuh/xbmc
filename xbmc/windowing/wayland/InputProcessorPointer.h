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

#include <cstdint>
#include <functional>

#include <wayland-client-protocol.hpp>

#include "input/XBMC_keysym.h"
#include "windowing/XBMC_events.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{

class IInputHandlerPointer
{
public:
  virtual void OnPointerEnter(wayland::pointer_t& pointer, std::uint32_t serial) {};
  virtual void OnPointerLeave() {};
  virtual void OnPointerEvent(XBMC_Event& event) = 0;
  virtual ~IInputHandlerPointer() = default;
};

class CInputProcessorPointer
{
public:
  CInputProcessorPointer(wayland::pointer_t const& pointer, IInputHandlerPointer& handler);
  void SetCoordinateScale(std::int32_t scale) { m_coordinateScale = scale; }

private:
  CInputProcessorPointer(CInputProcessorPointer const& other) = delete;
  CInputProcessorPointer& operator=(CInputProcessorPointer const& other) = delete;

  std::uint16_t ConvertMouseCoordinate(double coord);
  void SetMousePosFromSurface(double x, double y);
  void SendMouseMotion();
  void SendMouseButton(unsigned char button, bool pressed);

  wayland::pointer_t m_pointer;
  IInputHandlerPointer& m_handler;

  // Pointer position in *scaled* coordinates
  std::uint16_t m_pointerX{};
  std::uint16_t m_pointerY{};
  std::int32_t m_coordinateScale{1};
};

}
}
}