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

#include "Connection.h"

#include <cassert>

#include "utils/log.h"

using namespace KODI::WINDOWING::WAYLAND;

CConnection::CConnection(IConnectionHandler* handler)
: m_handler(handler)
{
  // TODO exception handling
  m_display.reset(new wayland::display_t);
    
  m_registry = m_display->get_registry();
  
  m_registry.on_global() = [this] (std::uint32_t name, std::string interface, std::uint32_t version)
  {
    // TODO Use constants here (integrate with waylandpp)
    if (interface == "wl_compositor")
    {
      std::uint32_t bindVersion = 3;
      CLog::Log(LOGDEBUG, "Binding Wayland protocol %s version %u (server has version %u)", interface.c_str(), bindVersion, version);
      m_registry.bind(name, m_compositor, bindVersion);
    }
    else if (interface == "wl_shell")
    {
      std::uint32_t bindVersion = 1;
      CLog::Log(LOGDEBUG, "Binding Wayland protocol %s version %u (server has version %u)", interface.c_str(), bindVersion, version);
      m_registry.bind(name, m_shell, bindVersion);
    }
    else if (interface == "wl_shm")
    {
      std::uint32_t bindVersion = 1;
      CLog::Log(LOGDEBUG, "Binding Wayland protocol %s version %u (server has version %u)", interface.c_str(), bindVersion, version);
      m_registry.bind(name, m_shm, bindVersion);
    }
    else if (interface == "wl_seat")
    {
      std::uint32_t bindVersion = 5;
      CLog::Log(LOGDEBUG, "Binding Wayland protocol %s version %u (server has version %u)", interface.c_str(), bindVersion, version);
      wayland::seat_t seat;
      m_registry.bind(name, seat, bindVersion);
      m_handler->OnSeatAdded(name, seat);
    }
  };
  m_registry.on_global_remove() = [this] (std::uint32_t name)
  {
    m_handler->OnSeatRemoved(name);
  };
  
  CLog::Log(LOGDEBUG, "Wayland connection: Waiting for global interfaces");
  m_display->roundtrip();
  CLog::Log(LOGDEBUG, "Wayland connection: Initial roundtrip complete");
  
  if (!m_compositor)
  {
    throw std::runtime_error("Missing required wl_compositor protocol");
  }
  if (!m_shell)
  {
    throw std::runtime_error("Missing required wl_shell protocol");
  }
  if (!m_shm)
  {
    throw std::runtime_error("Missing required wl_shm protocol");
  }
}

wayland::display_t& CConnection::GetDisplay()
{
  assert(m_display);
  return *m_display;
}

wayland::compositor_t& CConnection::GetCompositor()
{
  assert(m_compositor);
  return m_compositor;
}

wayland::shell_t& CConnection::GetShell()
{
  assert(m_shell);
  return m_shell;
}

wayland::shm_t& CConnection::GetShm()
{
  assert(m_shm);
  return m_shm;
}

