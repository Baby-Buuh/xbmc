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

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{
namespace PROTOCOL
{

class CSurface : public CProtocolBase<wl_surface>
{
public:
  using CProtocolBase::CProtocolBase;
  
  ~CSurface()
  {
    wl_surface_destroy(m_native);
  }

  wl_callback* CreateFrameCallback()
  {
    return wl_surface_frame(m_native);
  }

  void SetOpaqueRegion(wl_region* region)
  {
    wl_surface_set_opaque_region(m_native, region);
  }

  void Commit()
  {
    wl_surface_commit(m_native);
  }
};

}
}
}
}