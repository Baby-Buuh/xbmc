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

#include <map>
#include <mutex>
#include <set>

#include <wayland-client.hpp>
#include <wayland-cursor.hpp>

#include "Connection.h"
#include "SeatInputProcessor.h"
#include "windowing/WinSystem.h"

class IDispResource;

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{

class CWinSystemWayland : public CWinSystemBase, public IInputHandler, public IConnectionHandler
{
public:
  CWinSystemWayland();
  virtual ~CWinSystemWayland();

  bool InitWindowSystem() override;
  bool DestroyWindowSystem() override;

  bool CreateNewWindow(const std::string& name,
                       bool fullScreen,
                       RESOLUTION_INFO& res,
                       PHANDLE_EVENT_FUNC userFunction) override;

  bool DestroyWindow() override;

  bool ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop) override;
  bool SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays) override;

  void UpdateResolutions() override;

  bool Hide() override;
  bool Show(bool raise = true) override;
  
  void ShowOSMouse(bool show) override;
  
  virtual void Register(IDispResource *resource);
  virtual void Unregister(IDispResource *resource);
  
  // IInputHandler
  void OnEnter(std::uint32_t seatGlobalName, InputType type) override;
  void OnLeave(std::uint32_t seatGlobalName, InputType type) override;
  void OnEvent(std::uint32_t seatGlobalName, InputType type, XBMC_Event& event) override;
  void OnSetCursor(wayland::pointer_t& pointer, std::uint32_t serial) override;

  // IConnectionHandler
  void OnSeatAdded(std::uint32_t name, wayland::seat_t& seat) override;
  void OnGlobalRemoved(std::uint32_t name) override;

protected:
  void LoadDefaultCursor();
  void SendFocusChange(bool focus);
  
  std::unique_ptr<CConnection> m_connection;
  wayland::surface_t m_surface;
  wayland::shell_surface_t m_shellSurface;
  
  std::map<std::uint32_t, CSeatInputProcessor> m_seatProcessors;
  
  bool m_osCursorVisible = true;
  wayland::cursor_theme_t m_cursorTheme;
  wayland::buffer_t m_cursorBuffer;
  wayland::cursor_image_t m_cursorImage;
  wayland::surface_t m_cursorSurface;
  
  std::set<IDispResource*> m_dispResources;
  std::mutex m_dispResourcesMutex;
};


}
}
}