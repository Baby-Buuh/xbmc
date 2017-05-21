/*
 *      Copyright (C) 2011-2013 Team XBMC
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

#include <system_error>

using namespace KODI::WINDOWING::WAYLAND::PROTOCOL;

const wl_registry_listener CRegistry::ms_registry_listener = {
  CRegistry::HandleGlobalCallback,
  CRegistry::HandleRemoveGlobalCallback
};

/* We inject an IWaylandRegistration here which is a virtual
 * class which a callback for the global objects
 * used by xbmc. Once one of those objects becomes
 * available, we call the callback function on that
 * interface. If it returns false, then it means that the main
 * xbmc implementation isn't interested in that object, so we 
 * call out to a listener that can be bound to by any client code
 * (as it is a singleton) to see if that code is interested
 * in the interface and wants to bind to it. This is particularly
 * useful for testing purposes where custom objects on the compositor
 * side are used. */
CRegistry::CRegistry(wl_display* display,
                     IGlobalInterfaceHandler * const handler) :
m_handler(handler)
{
  m_registry = wl_display_get_registry(display);
  if (!m_registry)
  {
    throw std::system_error(errno, std::generic_category(), "Could not get registry from wl_display");
  }

  if (wl_registry_add_listener(m_registry, &ms_registry_listener, reinterpret_cast<void *> (this)) < 0)
  {
    throw std::runtime_error("Could not add listener to registry");
  }
}

CRegistry::~CRegistry()
{
  wl_registry_destroy(m_registry);
}

void
CRegistry::HandleGlobal(std::uint32_t name,
                        const char* interface,
                        std::uint32_t version)
{
  m_handler->OnGlobalInterfaceAvailable(name, interface, version);
}

void
CRegistry::HandleRemoveGlobal(std::uint32_t name)
{
  m_handler->OnGlobalInterfaceRemoved(name);
}

void* CRegistry::BindInternal(std::uint32_t name, const wl_interface* interface, std::uint32_t version)
{
  void* instance = wl_registry_bind(m_registry, name, interface, version);
  if (!instance)
  {
    throw std::system_error(errno, std::generic_category(), "Could not bind global Wayland protocol instance");
  }
  return instance;
}

void
CRegistry::HandleGlobalCallback(void* data,
                                wl_registry* registry,
                                std::uint32_t name,
                                const char* interface,
                                std::uint32_t version)
{
  static_cast<CRegistry*> (data)->HandleGlobal(name, interface, version);
}

void
CRegistry::HandleRemoveGlobalCallback(void* data,
                                      struct wl_registry* registry,
                                      std::uint32_t name)
{
  static_cast<CRegistry*> (data)->HandleRemoveGlobal(name);
}
