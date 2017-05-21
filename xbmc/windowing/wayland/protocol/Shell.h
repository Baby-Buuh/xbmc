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

#include <wayland-client.h>

#include "ProtocolBase.h"
#include "ShellSurface.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{
namespace PROTOCOL
{

class CShell : public CProtocolBase<wl_shell>
{
public:
  static constexpr ProtocolTag Meta = ProtocolTag("wl_shell", 1, &wl_shell_interface);
  using CProtocolBase::CProtocolBase;

  ~CShell()
  {
    wl_shell_destroy(m_native);
  }

  CShellSurface* CreateShellSurface(wl_surface* surface)
  {
    return new CShellSurface(wl_shell_get_shell_surface(m_native, surface));
  }
};

}
}
}
}
