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

#include "protocol/Connection.h"
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

}

bool CWinSystemWayland::InitWindowSystem()
{
  CLog::LogFunction(LOGINFO, "CWinSystemWayland::InitWindowSystem", "Connecting to Wayland server");
  m_connection.reset(new PROTOCOL::CConnection);
  return CWinSystemBase::InitWindowSystem();
}

bool CWinSystemWayland::DestroyWindowSystem()
{
  return true;
}

bool CWinSystemWayland::CreateNewWindow(const std::string& name,
                                    bool fullScreen,
                                    RESOLUTION_INFO& res,
                                    PHANDLE_EVENT_FUNC userFunction)
{
  return true;
}

bool CWinSystemWayland::DestroyWindow()
{
  return true;
}

void CWinSystemWayland::UpdateResolutions()
{
  CWinSystemBase::UpdateResolutions();


  CDisplaySettings::GetInstance().ClearCustomResolutions();
  CDisplaySettings::GetInstance().ApplyCalibrations();
}

bool CWinSystemWayland::ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop)
{
  return true;
}

bool CWinSystemWayland::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
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
