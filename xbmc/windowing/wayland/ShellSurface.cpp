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

using namespace KODI::WINDOWING::WAYLAND;

IShellSurface::ConfigureHandler& IShellSurface::OnConfigure()
{
  return m_onConfigure;
}

void IShellSurface::InvokeOnConfigure(std::uint32_t serial, CSizeInt size, StateBitset state)
{
  if (m_onConfigure)
  {
    m_onConfigure(serial, size, state);
  }
}
