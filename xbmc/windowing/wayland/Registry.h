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
#include <map>

#include <wayland-client-protocol.hpp>

#include "Connection.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{

/**
 * Handle Wayland globals
 *
 * Request singletons (bound once) with \ref RequestSingleton, non-singletons
 * such as wl_output with \ref Request, then call \ref Bind once.
 *
 * Make sure to destroy all registries before destroying the \ref CConnection.
 */
class CRegistry
{
public:
  explicit CRegistry(CConnection& connection) noexcept;

  /**
   * Request a static singleton global to be bound to a proxy
   *
   * You should only use this if the singleton is announced at registry bind time,
   * not dynamically. Use \ref Request for dynamically appearing objects,
   * even if they are only bound once.
   *
   * Cannot be called after \ref Bind has been called.
   *
   * \param target target of waylandpp proxy type
   * \param minVersion minimum version to bind
   * \param maxVersion maximum version to bind
   * \param required whether to throw an exception when the bind cannot be satisfied
   *                 by the compositor - exception is thrown in \ref Bind
   */
  template<typename T>
  void RequestSingleton(T& target, std::uint32_t minVersion, std::uint32_t maxVersion, bool required = true)
  {
    RequestSingletonInternal(target, T::interface_name, minVersion, maxVersion, required);
  }
  using AddHandler = std::function<void(std::uint32_t /* name */, wayland::proxy_t&& /* object */)>;
  using RemoveHandler = std::function<void(std::uint32_t) /* name */>;
  /**
   * Request a callback when a dynamic global appears or disappears
   *
   * The callbacks may be called during or after \ref Bind from the thread
   * that calls \ref Bind or the global Wayland message pump thread.
   *
   * Cannot be called after \ref Bind has been called.
   *
   * \param minVersion minimum version to bind
   * \param maxVersion maximum version to bind
   * \param addHandler function that is called when a global of the requested
   *                   type is added
   * \param removeHandler function that is called when a global of the requested
   *                      type is removed
   */
  template<typename T>
  void Request(std::uint32_t minVersion, std::uint32_t maxVersion, AddHandler addHandler, RemoveHandler removeHandler)
  {
    RequestInternal([]{ return T(); }, T::interface_name, minVersion, maxVersion, addHandler, removeHandler);
  }

  /**
   * Create a registry object at the compositor and do an roundtrip to bind
   * objects
   *
   * This function blocks until the initial roundtrip is complete. All statically
   * requested singletons that were available will be bound then.
   *
   * Neither statically nor dynamically requested proxies will be bound before this
   * function is called.
   *
   * May throw std::runtime_error if a required global was not found.
   *
   * Can only be called once for the same \ref CRegistry object.
   */
  void Bind();
  /**
   * Unbind all singletons requested with \ref RequestSingleton
   */
  void UnbindSingletons();

private:
  CRegistry(CRegistry const& other) = delete;
  CRegistry& operator=(CRegistry const& other) = delete;

  void RequestSingletonInternal(wayland::proxy_t& target, std::string const& interfaceName, std::uint32_t minVersion, std::uint32_t maxVersion, bool required);
  void RequestInternal(std::function<wayland::proxy_t()> constructor, std::string const& interfaceName, std::uint32_t minVersion, std::uint32_t maxVersion, AddHandler addHandler, RemoveHandler removeHandler);
  void CheckRequired();

  CConnection& m_connection;
  wayland::registry_t m_registry;

  struct SingletonBindInfo
  {
    wayland::proxy_t& target;
    // Throw exception if trying to bind below this version and required
    std::uint32_t minVersion;
    // Limit bind version to the minimum of this and compositor version
    std::uint32_t maxVersion;
    bool required;
    SingletonBindInfo(wayland::proxy_t& target, std::uint32_t minVersion, std::uint32_t maxVersion, bool required)
    : target{target}, minVersion{minVersion}, maxVersion{maxVersion}, required{required}
    {}
  };
  std::map<std::string, SingletonBindInfo> m_singletonBinds;

  struct BindInfo
  {
    std::function<wayland::proxy_t()> constructor;
    std::uint32_t minVersion;
    std::uint32_t maxVersion;
    AddHandler addHandler;
    RemoveHandler removeHandler;
    BindInfo(std::function<wayland::proxy_t()> constructor, std::uint32_t minVersion, std::uint32_t maxVersion, AddHandler addHandler, RemoveHandler removeHandler)
    : constructor{constructor}, minVersion{minVersion}, maxVersion{maxVersion}, addHandler{addHandler}, removeHandler{removeHandler}
    {}
  };
  std::map<std::string, BindInfo> m_binds;

  std::map<std::uint32_t, std::reference_wrapper<BindInfo>> m_boundNames;
};

}
}
}