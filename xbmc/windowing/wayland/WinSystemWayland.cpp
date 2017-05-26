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

#include "WinSystemWayland.h"

#include "settings/DisplaySettings.h"
#include "utils/log.h"

using namespace KODI::WINDOWING::WAYLAND;

CWinSystemWayland::CWinSystemWayland() :
CWinSystemBase()
{
  m_eWindowSystem = WINDOW_SYSTEM_WAYLAND;
}

CWinSystemWayland::~CWinSystemWayland()
{
  DestroyWindowSystem();
}

bool CWinSystemWayland::InitWindowSystem()
{
  CLog::LogFunction(LOGINFO, "CWinSystemWayland::InitWindowSystem", "Connecting to Wayland server");
  m_connection.reset(new CConnection);
  return CWinSystemBase::InitWindowSystem();
}

bool CWinSystemWayland::DestroyWindowSystem()
{
  m_shellSurface = wayland::shell_surface_t();
  m_surface = wayland::surface_t();
  m_connection.reset();
  return true;
}

bool CWinSystemWayland::CreateNewWindow(const std::string& name,
                                        bool fullScreen,
                                        RESOLUTION_INFO& res,
                                        PHANDLE_EVENT_FUNC userFunction)
{
  m_surface = m_connection->GetCompositor().create_surface();
  m_shellSurface = m_connection->GetShell().get_shell_surface(m_surface);
  m_shellSurface.set_class("kodi");
  m_shellSurface.set_title("Kodi");
  m_shellSurface.set_toplevel();
  
  return true;
}

bool CWinSystemWayland::DestroyWindow()
{
  return true;
}

void CWinSystemWayland::UpdateResolutions()
{
  CWinSystemBase::UpdateResolutions();

  // FIXME
  UpdateDesktopResolution(CDisplaySettings::GetInstance().GetResolutionInfo(RES_DESKTOP), 0, 1280, 720, 60);
  //CDisplaySettings::GetInstance().GetResolutionInfo(RES_DESKTOP).strId = std::to_string(d);
//      SetWindowResolution(width, height);

  CDisplaySettings::GetInstance().ClearCustomResolutions();
  CDisplaySettings::GetInstance().ApplyCalibrations();
}

bool CWinSystemWayland::ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop)
{
  return true;
}

bool CWinSystemWayland::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
  // FIXME
  m_nWidth      = res.iWidth;
  m_nHeight     = res.iHeight;
  m_bFullScreen = fullScreen;
  
  return true;
}

bool CWinSystemWayland::Hide()
{
  return false;
}

bool CWinSystemWayland::Show(bool raise)
{
  return true;
}

void CWinSystemWayland::Register(IDispResource * /*resource*/)
{
}

void CWinSystemWayland::Unregister(IDispResource * /*resource*/)
{
}
