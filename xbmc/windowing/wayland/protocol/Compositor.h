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
#include "Surface.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{
namespace PROTOCOL
{

class CCompositor : public CProtocolBase<wl_compositor>
{
public:
  static constexpr ProtocolTag Meta = ProtocolTag("wl_compositor", 1, &wl_compositor_interface);
  using CProtocolBase::CProtocolBase;

  ~CCompositor()
  {
    wl_compositor_destroy(m_native);
  }

  CSurface* CreateSurface() const
  {
    return new CSurface(wl_compositor_create_surface(m_native));
  }

  wl_region * CreateRegion() const
  {
    return wl_compositor_create_region(m_native);
  }
};

}
}
}
}
