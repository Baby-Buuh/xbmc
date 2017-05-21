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

#include "Types.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{
namespace PROTOCOL
{

class CCompositor final
{
public:
  static constexpr ProtocolTag Meta = ProtocolTag("wl_compositor", 1, &wl_compositor_interface);
  typedef wl_compositor NativeType;

  explicit CCompositor(NativeType* compositor) :
  m_compositor(compositor)
  {
  }

  ~CCompositor()
  {
    wl_compositor_destroy(m_compositor);
  }

  CCompositor(const CCompositor &) = delete;
  CCompositor &operator=(const CCompositor &) = delete;

  NativeType * GetWlCompositor()
  {
    return m_compositor;
  }

  wl_surface * CreateSurface() const
  {
    return wl_compositor_create_surface(m_compositor);
  }

  wl_region * CreateRegion() const
  {
    return wl_compositor_create_region(m_compositor);
  }

private:
  NativeType* m_compositor;
};

}
}
}
}
