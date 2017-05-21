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
#include <wayland-client.h>

#include "Compositor.h"
#include "Registry.h"
#include "Shell.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{
namespace PROTOCOL
{

class CConnection : IGlobalInterfaceHandler
{
public:
  explicit CConnection(std::string const & name = "");
  ~CConnection();

  CConnection(const CConnection &) = delete;
  CConnection& operator=(const CConnection &) = delete;

  wl_display* GetWlDisplay()
  {
    return m_display;
  }
  
  CCompositor& GetCompositor()
  {
    return *m_compositor;
  }
  
  CShell& GetShell()
  {
    return *m_shell;
  }

  /**
   * Create a sync callback object. This can be wrapped in a
   * \ref CCallback object to call an arbitrary function
   * as soon as the display has finished processing all commands.
   *
   * This does not block until a synchronization is complete.
   */
  wl_callback* Sync();
  
  // IGlobalInterfaceHandler
  bool OnGlobalInterfaceAvailable(std::uint32_t name, const std::string& interface, std::uint32_t version) override;
  void OnGlobalInterfaceRemoved(std::uint32_t name) override;

protected:
  template<typename T>
  bool TryBind(std::unique_ptr<T>& target, std::uint32_t name, std::string const & interface, std::uint32_t version);
  template<typename T>
  void ThrowIfUnbound(std::unique_ptr<T> const & instance);
  
private:
  wl_display* m_display = nullptr;
  std::unique_ptr<CRegistry> m_registry;
  
  std::unique_ptr<CCompositor> m_compositor;
  std::unique_ptr<CShell> m_shell;
  
  void Roundtrip();
};

}
}
}
}