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

#include <string>

#include <wayland-client.h>

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{
namespace PROTOCOL
{

class IGlobalInterfaceHandler
{
public:

  virtual ~IGlobalInterfaceHandler()
  {
  }
  virtual bool OnGlobalInterfaceAvailable(std::uint32_t name, std::string const & interface, std::uint32_t version) = 0;
  virtual void OnGlobalInterfaceRemoved(std::uint32_t name) = 0;
};

class CRegistry final
{
public:
  CRegistry(wl_display* display,
            IGlobalInterfaceHandler * const handler);

  ~CRegistry();

  CRegistry(const CRegistry &) = delete;
  CRegistry &operator=(const CRegistry &) = delete;

  wl_registry* GetWlRegistry();

  /**
   * Bind to an interface on the server
   * 
   * As wl_registry_bind returns an untyped void*, this convenience function
   * casts this pointer to the requested type. Be aware that there is no
   * type-checking involved, so you must make sure that your type is correct.
   */
  template<typename CreateT>
  CreateT Bind(std::uint32_t name,
               const wl_interface* interface,
               std::uint32_t version)
  {
    void* object = BindInternal(name,
                                interface,
                                version);
    return reinterpret_cast<CreateT> (object);
  }

private:
  wl_registry* m_registry = nullptr;
  IGlobalInterfaceHandler* m_handler = nullptr;

  /* Once a global becomes available, we immediately bind to it here
   * and then notify the injected listener interface that the global
   * is available on a named object. This allows that interface to
   * respond to the arrival of the new global how it wishes */
  void *BindInternal(std::uint32_t name,
                     const wl_interface* interface,
                     std::uint32_t version);

  void HandleGlobal(std::uint32_t name, const char * interface, std::uint32_t version);
  void HandleRemoveGlobal(std::uint32_t name);

  /**
   * Static C wrapper for calling \ref HandleGlobal
   * \param data CRegistry instance
   */
  static void HandleGlobalCallback(void * data, wl_registry * registry,
                                   std::uint32_t name, const char * interface, std::uint32_t version);
  /**
   * Static C wrapper for calling \ref HandleRemoveGlobal
   * \param data CRegistry instance
   */
  static void HandleRemoveGlobalCallback(void * data, wl_registry * registry,
                                         std::uint32_t name);
  static const wl_registry_listener ms_registry_listener;
};

}
}
}
}