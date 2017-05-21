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
#include <cstdint>

#include "ProtocolBase.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{
namespace PROTOCOL
{

class CShellSurface : public CProtocolBase<wl_shell_surface>
{
public:
  explicit CShellSurface(NativeType* shellSurface);

  ~CShellSurface()
  {
    wl_shell_surface_destroy(m_native);
  }

  void SetFullscreen(enum wl_shell_surface_fullscreen_method method,
                     std::uint32_t framerate,
                     struct wl_output *output)
  {
    wl_shell_surface_set_fullscreen(m_native,
                                    method,
                                    framerate,
                                    output);
  }

private:
  void HandlePing(std::uint32_t serial);
  void HandleConfigure(std::uint32_t edges,
                       std::int32_t width,
                       std::int32_t height);
  void HandlePopupDone();
  
  static const wl_shell_surface_listener ms_listener;

  static void HandlePingCallback(void *,
                                 wl_shell_surface *,
                                 std::uint32_t);
  static void HandleConfigureCallback(void *,
                                      wl_shell_surface *,
                                      std::uint32_t,
                                      std::int32_t,
                                      std::int32_t);
  static void HandlePopupDoneCallback(void *,
                                      wl_shell_surface *);
};

}
}
}
}