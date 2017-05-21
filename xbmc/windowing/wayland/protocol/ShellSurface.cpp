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

#include "ShellSurface.h"

#include <wayland-client.h>

using namespace KODI::WINDOWING::WAYLAND::PROTOCOL;

const wl_shell_surface_listener CShellSurface::ms_listener = {
  CShellSurface::HandlePingCallback,
  CShellSurface::HandleConfigureCallback,
  CShellSurface::HandlePopupDoneCallback
};

CShellSurface::CShellSurface(wl_shell_surface* shellSurface) :
CProtocolBase(shellSurface)
{
  wl_shell_surface_add_listener(m_native, &ms_listener,
                                reinterpret_cast<void *> (this));
}

void
CShellSurface::HandlePingCallback(void* data,
                                  wl_shell_surface*,
                                  std::uint32_t serial)
{
  return static_cast<CShellSurface *> (data)->HandlePing(serial);
}

void
CShellSurface::HandleConfigureCallback(void* data,
                                       wl_shell_surface*,
                                       std::uint32_t edges,
                                       std::int32_t width,
                                       std::int32_t height)
{
  return static_cast<CShellSurface *> (data)->HandleConfigure(edges,
                                                              width,
                                                              height);
}

void
CShellSurface::HandlePopupDoneCallback(void* data,
                                       wl_shell_surface*)
{
  return static_cast<CShellSurface *> (data)->HandlePopupDone();
}

void
CShellSurface::HandlePing(std::uint32_t serial)
{
  wl_shell_surface_pong(m_native, serial);
}

void
CShellSurface::HandleConfigure(std::uint32_t edges,
                               std::int32_t width,
                               std::int32_t height)
{
}

void
CShellSurface::HandlePopupDone()
{
}
