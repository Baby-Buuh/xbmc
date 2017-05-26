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

#include <wayland-client.hpp>

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{

class CConnection
{
public:
  CConnection();
  
  wayland::display_t& GetDisplay();
  wayland::compositor_t& GetCompositor();
  wayland::shell_t& GetShell();
  
private: 
  std::unique_ptr<wayland::display_t> m_display;
  wayland::registry_t m_registry;
  wayland::compositor_t m_compositor;
  wayland::shell_t m_shell;
};

}
}
}