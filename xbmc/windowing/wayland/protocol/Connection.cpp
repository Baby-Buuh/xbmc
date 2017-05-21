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

#include <string>
#include <system_error>

#include "utils/log.h"

using namespace KODI::WINDOWING::WAYLAND::PROTOCOL;

CConnection::CConnection(std::string const & name)
{
  m_display = wl_display_connect(name.empty() ? nullptr : name.c_str());

  if (!m_display)
  {
    std::string realname = name;
    if (realname.empty())
    {
      // wl_display_connect falls back to WAYLAND_DISPLAY environment variable
      realname = getenv("WAYLAND_DISPLAY");
    }
    if (realname.empty())
    {
      // and finally to this constant
      realname = "wayland-0";
    }
    throw std::runtime_error(std::string("Could not connect to Wayland display \"" + realname + "\""));
  }

  m_registry.reset(new CRegistry(m_display, this));
  CLog::Log(LOGDEBUG, "Wayland connection: Waiting for global interfaces");
  Roundtrip();
  CLog::Log(LOGDEBUG, "Wayland connection: Initial roundtrip complete");

  // Verify
  ThrowIfUnbound(m_compositor);
  ThrowIfUnbound(m_shell);
}

CConnection::~CConnection()
{
  m_registry.reset();
  wl_display_disconnect(m_display);
}

template<typename T>
void CConnection::ThrowIfUnbound(const std::unique_ptr<T>& instance)
{
  if (!instance)
  {
    throw std::runtime_error(std::string("Kodi requires the Wayland protocol \"") + T::Meta.protocol + "\", but it was not announced by the compositor");
  }
}

wl_callback* CConnection::Sync()
{
  wl_callback* cb = wl_display_sync(m_display);
  if (!cb)
  {
    throw std::system_error(errno, std::generic_category(), "Could not create wl_display sync object");
  }
  return cb;
}

void CConnection::Roundtrip()
{
  // wl_display_roundtrip blocks and it may only be used as long as the event queue dispatcher is not running yet!
  if (wl_display_roundtrip(m_display) < 0)
  {
    throw std::runtime_error("Initial roundtrip to Wayland server for receiving global interfaces failed");
  }
}

template<typename T>
bool CConnection::TryBind(std::unique_ptr<T>& target, std::uint32_t name, const std::string& interface, std::uint32_t version)
{
  if (interface == T::Meta.protocol)
  {
    if (version >= T::Meta.version)
    {
      CLog::Log(LOGDEBUG, "Binding Wayland protocol %s version %u (server offered version %u)", interface.c_str(), T::Meta.version, version);
      target.reset(new T(m_registry->Bind<typename T::NativeType *>(name, T::Meta.interface, T::Meta.version)));
      return true;
    }
    else
    {
      throw std::runtime_error(std::string("Wayland compositor has protocol \"") + interface + "\" in version " + std::to_string(version) + ", but Kodi requires at least version " + std::to_string(T::Meta.version));
    }
  }
  else
  {
    return false;
  }
}

bool CConnection::OnGlobalInterfaceAvailable(std::uint32_t name, const std::string& interface, std::uint32_t version)
{
  return
  TryBind(m_compositor, name, interface, version)
    || TryBind(m_shell, name, interface, version);
}

void CConnection::OnGlobalInterfaceRemoved(std::uint32_t name)
{
  // This is only interesting for wl_output, other stuff should never disappear
}
