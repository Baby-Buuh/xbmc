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

#include "Registry.h"

#include <wayland-client-protocol.h>

#include "utils/log.h"

using namespace KODI::WINDOWING::WAYLAND;

namespace
{

void TryBind(wayland::registry_t& registry, wayland::proxy_t& target, std::uint32_t name, std::string const& interface, std::uint32_t minVersion, std::uint32_t maxVersion, std::uint32_t offeredVersion)
{
  if (offeredVersion < minVersion)
  {
    CLog::Log(LOGWARNING, "Not binding Wayland protocol %s because server has only version %u (we need at least version %u)", interface.c_str(), offeredVersion, minVersion);
  }
  else
  {
    // Binding below the offered version is OK
    auto bindVersion = std::min(maxVersion, offeredVersion);
    CLog::Log(LOGDEBUG, "Binding Wayland protocol %s version %u (server has version %u)", interface.c_str(), bindVersion, offeredVersion);
    registry.bind(name, target, bindVersion);
  }
}

}

CRegistry::CRegistry(CConnection& connection) noexcept
: m_connection{connection}
{
}

void CRegistry::RequestSingletonInternal(wayland::proxy_t& target, std::string const& interfaceName, std::uint32_t minVersion, std::uint32_t maxVersion, bool required)
{
  if (m_registry)
  {
    throw std::logic_error("Cannot request more binds from registry after binding has started");
  }
  m_singletonBinds.emplace(std::piecewise_construct, std::forward_as_tuple(interfaceName), std::forward_as_tuple(target, minVersion, maxVersion, required));
}

void CRegistry::RequestInternal(std::function<wayland::proxy_t()> constructor, const std::string& interfaceName, std::uint32_t minVersion, std::uint32_t maxVersion, AddHandler addHandler, RemoveHandler removeHandler)
{
  if (m_registry)
  {
    throw std::logic_error("Cannot request more binds from registry after binding has started");
  }
  m_binds.emplace(std::piecewise_construct, std::forward_as_tuple(interfaceName), std::forward_as_tuple(constructor, minVersion, maxVersion, addHandler, removeHandler));
}

void CRegistry::Bind()
{
  if (m_registry)
  {
    throw std::logic_error("Cannot start binding on registry twice");
  }

  // We want to block in this function until we have received the global interfaces
  // from the compositor - no matter whether the global event pump is running
  // or not.
  // If it is running, we have to take special precautions not to drop events between
  // the creation of the registry and attaching event handlers, so we create
  // an extra queue and use that to dispatch the singleton globals. Then
  // we switch back to the global queue for further dispatch of interfaces
  // added/removed dynamically.

  auto registryRoundtripQueue = m_connection.GetDisplay().create_queue();

  // TODO create API for proxy wrappers in waylandpp
  auto displayProxy = static_cast<wl_display*> (wl_proxy_create_wrapper(m_connection.GetDisplay()));
  wl_proxy_set_queue(reinterpret_cast<wl_proxy*> (displayProxy), registryRoundtripQueue);
  auto wlRegistry = wl_display_get_registry(displayProxy);
  wl_proxy_wrapper_destroy(displayProxy);
  wayland::proxy_t registryProxy(reinterpret_cast<wl_proxy*> (wlRegistry));

  m_registry = wayland::registry_t(registryProxy);

  m_registry.on_global() = [this] (std::uint32_t name, std::string interface, std::uint32_t version)
  {
    {
      auto it = m_singletonBinds.find(interface);
      if (it != m_singletonBinds.end())
      {
        auto& bind = it->second;
        TryBind(m_registry, bind.target, name, interface, bind.minVersion, bind.maxVersion, version);
        if (bind.target)
        {
          // Events on the bound global should always go to the main queue
          // TODO Figure out what to do with globals that announce some initial events,
          // those could get lost when the event pump is running
          bind.target.set_queue(wayland::event_queue_t());
        }
        return;
      }
    }

    {
      auto it = m_binds.find(interface);
      if (it != m_binds.end())
      {
        auto& bind = it->second;
        wayland::proxy_t target{bind.constructor()};
        TryBind(m_registry, target, name, interface, bind.minVersion, bind.maxVersion, version);
        if (target)
        {
          // Events on the bound global should always go to the main queue
          target.set_queue(wayland::event_queue_t());
          m_boundNames.emplace(name, bind);
          bind.addHandler(name, std::move(target));
        }
        return;
      }
    }
  };

  m_registry.on_global_remove() = [this] (std::uint32_t name)
  {
    auto it = m_boundNames.find(name);
    if (it != m_boundNames.end())
    {
      m_boundNames.erase(it);
      it->second.get().removeHandler(name);
    }
  };

  CLog::Log(LOGDEBUG, "Wayland connection: Waiting for global interfaces");
  m_connection.GetDisplay().roundtrip_queue(registryRoundtripQueue);
  CLog::Log(LOGDEBUG, "Wayland connection: Roundtrip complete");

  CheckRequired();

  // FIXME This needs more work, events could be dispatched in parallel!
  // Now switch it to the global queue for further runtime binds
  m_registry.set_queue(wayland::event_queue_t());
  // Roundtrip extra queue one last time in case something got queued up there
  m_connection.GetDisplay().roundtrip_queue(registryRoundtripQueue);
}

void CRegistry::UnbindSingletons()
{
  for (auto& bind : m_singletonBinds)
  {
    bind.second.target.proxy_release();
  }
}

void CRegistry::CheckRequired()
{
  for (auto const& bind : m_singletonBinds)
  {
    if (bind.second.required && !bind.second.target)
    {
      throw std::runtime_error(std::string("Missing required ") + bind.first + " protocol");
    }
  }
}