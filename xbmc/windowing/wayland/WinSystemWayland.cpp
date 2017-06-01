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

#include <algorithm>
#include <limits>

#include "Connection.h"
#include "guilib/DispResource.h"
#include "guilib/LocalizeStrings.h"
#include "settings/DisplaySettings.h"
#include "utils/log.h"
#include "WinEventsWayland.h"
#include "utils/StringUtils.h"
#include "guilib/GraphicContext.h"

using namespace KODI::WINDOWING::WAYLAND;
using namespace std::placeholders;

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
  m_connection.reset(new CConnection(this));
  if (m_seatProcessors.empty())
  {
    CLog::Log(LOGWARNING, "Wayland compositor did not announce a wl_seat - you will not have any input devices for the time being");
  }
  // Do another roundtrip to get initial wl_output information
  m_connection->GetDisplay().roundtrip();
  CWinEventsWayland::SetDisplay(&m_connection->GetDisplay());
  return CWinSystemBase::InitWindowSystem();
}

bool CWinSystemWayland::DestroyWindowSystem()
{
  DestroyWindow();
  // wl_display_disconnect frees all proxy objects, so we have to make sure
  // all stuff is gone on the C++ side before that
  m_cursorSurface = wayland::surface_t();
  m_cursorBuffer = wayland::buffer_t();
  m_cursorImage = wayland::cursor_image_t();
  m_cursorTheme = wayland::cursor_theme_t();
  m_seatProcessors.clear();
  m_outputs.clear();
  CWinEventsWayland::SetDisplay(nullptr);
  
  m_connection.reset();
  return CWinSystemBase::DestroyWindowSystem();
}

bool CWinSystemWayland::CreateNewWindow(const std::string& name,
                                        bool fullScreen,
                                        RESOLUTION_INFO& res,
                                        PHANDLE_EVENT_FUNC userFunction)
{
  m_surface = m_connection->GetCompositor().create_surface();
  m_shellSurface = m_connection->GetShell().get_shell_surface(m_surface);
  // "class" should match the name of the .desktop file; it's needed for correct
  // interaction with WM menus and displaying the window icon in the app list
  m_shellSurface.set_class("kodi");
  m_shellSurface.set_title(name);
  m_shellSurface.on_ping() = std::bind(&wayland::shell_surface_t::pong, &m_shellSurface, _1);
  m_shellSurface.on_configure() = std::bind(&CWinSystemWayland::HandleSurfaceConfigure, this, _1, _2, _3);

  return true;
}

bool CWinSystemWayland::DestroyWindow()
{
  m_shellSurface = wayland::shell_surface_t();
  // waylandpp automatically calls wl_surface_destroy when the last reference is removed
  m_surface = wayland::surface_t();

  return true;
}

bool CWinSystemWayland::CanDoWindowed()
{
  return false;
}

int CWinSystemWayland::GetNumScreens()
{
  return m_outputs.size();
}

int CWinSystemWayland::GetCurrentScreen()
{
  return m_currentScreen;
}

void CWinSystemWayland::UpdateResolutions()
{
  CWinSystemBase::UpdateResolutions();

  CDisplaySettings::GetInstance().ClearCustomResolutions();

  // FIXME why does X11 pretend to have only one screen?
  unsigned int screenIdx = 0;

  std::vector<RESOLUTION_INFO> desktopResolutions, additionalResolutions;

  // Collect resolutions into two pairs:
  // - Currently active resolutions (used to set RES_DESKTOP + screenIdx)
  // - Additional resolutions that are added subsequently
  for (auto const& outputPair : m_outputs)
  {
    auto const& output = outputPair.second;
    std::string outputName = UserFriendlyOutputName(output);
    auto const& modes = output.GetModes();
    auto const& currentMode = output.GetCurrentMode();
    auto physicalSize = output.GetPhysicalSize();
    CLog::Log(LOGINFO, "Output \"%s\" size %dx%d mm index %u has %zu mode(s)", outputName.c_str(), std::get<0>(physicalSize), std::get<1>(physicalSize), screenIdx, modes.size());

    for (auto const& mode : modes)
    {
      bool isCurrent = (mode == currentMode);
      float pixelRatio = output.GetPixelRatioForMode(mode);
      CLog::Log(LOGINFO, "- %dx%d @%.3f Hz pixel ratio %.3f%s", mode.width, mode.height, mode.refreshMilliHz / 1000.0f, pixelRatio, isCurrent ? " current" : "");

      RESOLUTION_INFO res(mode.width, mode.height);
      res.bFullScreen = true;
      res.iScreen = screenIdx;
      res.strOutput = outputName;
      res.fPixelRatio = pixelRatio;
      res.fRefreshRate = mode.refreshMilliHz / 1000.0f;
      g_graphicsContext.ResetOverscan(res);

      if (isCurrent)
      {
        desktopResolutions.emplace_back(std::move(res));
      }
      else
      {
        additionalResolutions.emplace_back(std::move(res));
      }
    }

    screenIdx++;
  }

  // Now update desktop resolutions
  for (auto const& desktopResolution : desktopResolutions)
  {
    std::size_t index = RES_DESKTOP + desktopResolution.iScreen;
    if (index >= CDisplaySettings::GetInstance().ResolutionInfoSize())
    {
      // add new empty resolution
      RESOLUTION_INFO res;
      CDisplaySettings::GetInstance().AddResolutionInfo(res);
    }
    CDisplaySettings::GetInstance().GetResolutionInfo(index) = desktopResolution;
  }

  // And finally add additional resolutions
  for (auto const& additionalResolution : additionalResolutions)
  {
    CDisplaySettings::GetInstance().AddResolutionInfo(additionalResolution);
  }

  CDisplaySettings::GetInstance().ApplyCalibrations();
}

bool CWinSystemWayland::ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop)
{
  // Windowed mode is unsupported
  return false;
}

bool CWinSystemWayland::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
  CLog::Log(LOGINFO, "Wayland trying to switch mode to %dx%d @%.3f Hz on output \"%s\"", res.iWidth, res.iHeight, res.fRefreshRate, res.strOutput.c_str());

  // Try to match output
  wayland::output_t output;
  auto outputIt = std::find_if(m_outputs.cbegin(), m_outputs.cend(),
                               [this,&res](decltype(m_outputs)::value_type const& entry)
                               {
                                 return (res.strOutput == UserFriendlyOutputName(entry.second));
                               });
  if (outputIt != m_outputs.end())
  {
    output = outputIt->second.GetWaylandOutput();
    CLog::Log(LOGDEBUG, "Resolved output \"%s\" to bound Wayland global %u", res.strOutput.c_str(), outputIt->second.GetGlobalName());
  }
  else
  {
    CLog::Log(LOGINFO, "Could not match output \"%s\" to a currently available Wayland output, falling back to default output", res.strOutput.c_str());
  }

  m_nWidth = res.iWidth;
  m_nHeight = res.iHeight;
  m_bFullScreen = fullScreen;
  // This is just a guess since the compositor is free to ignore our frame rate
  // request
  m_fRefreshRate = res.fRefreshRate;
  // There is -no- guarantee that the compositor will put the surface on this
  // screen, but pretend that it does so we have any information at all
  m_currentScreen = res.iScreen;
  
  if (fullScreen)
  {
    CLog::Log(LOGDEBUG, "Set fullscreen on output %d class %s", output.get_id(), output.get_class().c_str());
    m_shellSurface.set_fullscreen(wayland::shell_surface_fullscreen_method::driver, res.fRefreshRate * 1000, output);
  }
  else
  {
    // Shouldn't happen since we claim not to support windowed modes
    CLog::Log(LOGWARNING, "Wayland windowing system asked to switch to windowed mode which is not really supported");
    m_shellSurface.set_toplevel();
  }

  return true;
}

void CWinSystemWayland::HandleSurfaceConfigure(wayland::shell_surface_resize edges, std::int32_t width, std::int32_t height)
{
  CLog::Log(LOGINFO, "Got Wayland surface size %dx%d", width, height);
  
  m_nWidth = width;
  m_nHeight = height;
  
  // Mark everything opaque so the compositor can render it faster
  wayland::region_t opaqueRegion = m_connection->GetCompositor().create_region();
  opaqueRegion.add(0, 0, width, height);
  m_surface.set_opaque_region(opaqueRegion);
  // No surface commit, EGL context will do that when it changes the buffer
}

std::string CWinSystemWayland::UserFriendlyOutputName(const COutput& output)
{
  std::vector<std::string> parts;
  if (!output.GetMake().empty())
  {
    parts.emplace_back(output.GetMake());
  }
  if (!output.GetModel().empty())
  {
    parts.emplace_back(output.GetModel());
  }
  if (parts.empty())
  {
    // Fallback to "unknown" if no name received from compositor
    parts.emplace_back(g_localizeStrings.Get(13205));
  }

  // Add position
  std::int32_t x, y;
  std::tie(x, y) = output.GetPosition();
  if (x != 0 || y != 0)
  {
    parts.emplace_back(StringUtils::Format("@{}x{}", x, y));
  }

  return StringUtils::Join(parts, " ");
}

bool CWinSystemWayland::Hide()
{
  // wl_shell_surface does not really support this - wait for xdg_shell
  return false;
}

bool CWinSystemWayland::Show(bool raise)
{
  // wl_shell_surface does not really support this - wait for xdg_shell
  return true;
}

bool CWinSystemWayland::HasCursor()
{
  return std::any_of(m_seatProcessors.cbegin(), m_seatProcessors.cend(),
                     [](decltype(m_seatProcessors)::value_type const& entry)
                     {
                       return entry.second.HasPointerCapability();
                     });
}

void CWinSystemWayland::ShowOSMouse(bool show)
{
  m_osCursorVisible = show;
}

void CWinSystemWayland::LoadDefaultCursor()
{
  if (!m_cursorSurface)
  {
    // Load default cursor theme and default cursor
    // Size of 16px is somewhat random
    // Cursor theme must be kept around since the lifetime of the image buffers
    // is coupled to it
    m_cursorTheme = wayland::cursor_theme_t("", 16, m_connection->GetShm());
    wayland::cursor_t cursor;
    try
    {
      cursor = m_cursorTheme.get_cursor("default");
    }
    catch (std::exception& e)
    {
      CLog::Log(LOGWARNING, "Could not load default cursor from theme, continuing without OS cursor");
    }
    // Just use the first image, do not handle animation
    m_cursorImage = cursor.image(0);
    m_cursorBuffer = m_cursorImage.get_buffer();
    m_cursorSurface = m_connection->GetCompositor().create_surface();
  }
  // Attach buffer to a surface - it seems that the compositor may change
  // the surface when the pointer leaves the surface, so we reattach the buffer each time
  m_cursorSurface.attach(m_cursorBuffer, 0, 0);
  m_cursorSurface.damage(0, 0, m_cursorImage.width(), m_cursorImage.height());
  m_cursorSurface.commit();
}

void CWinSystemWayland::Register(IDispResource* resource)
{
  std::lock_guard<decltype(m_dispResourcesMutex) > lock(m_dispResourcesMutex);
  m_dispResources.emplace(resource);
}

void CWinSystemWayland::Unregister(IDispResource* resource)
{
  std::lock_guard<decltype(m_dispResourcesMutex) > lock(m_dispResourcesMutex);
  m_dispResources.erase(resource);
}

void CWinSystemWayland::OnSeatAdded(std::uint32_t name, wayland::seat_t& seat)
{
  m_seatProcessors.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(name, seat, this));
}

void CWinSystemWayland::OnOutputAdded(std::uint32_t name, wayland::output_t& output)
{
  m_outputs.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(name, output));
}

void CWinSystemWayland::OnGlobalRemoved(std::uint32_t name)
{
  m_seatProcessors.erase(name);
  if (m_outputs.erase(name) != 0)
  {
    // TODO Handle: Update resolution etc.
  }
}

void CWinSystemWayland::SendFocusChange(bool focus)
{
  std::lock_guard<decltype(m_dispResourcesMutex) > lock(m_dispResourcesMutex);
  for (auto dispResource : m_dispResources)
  {
    dispResource->OnAppFocusChange(focus);
  }
}

void CWinSystemWayland::OnEnter(std::uint32_t seatGlobalName, InputType type)
{
  // Couple to keyboard focus
  if (type == InputType::KEYBOARD)
  {
    SendFocusChange(true);
  }
}

void CWinSystemWayland::OnLeave(std::uint32_t seatGlobalName, InputType type)
{
  // Couple to keyboard focus
  if (type == InputType::KEYBOARD)
  {
    SendFocusChange(false);
  }
}

void CWinSystemWayland::OnEvent(std::uint32_t seatGlobalName, InputType type, XBMC_Event& event)
{
  CWinEvents::MessagePush(&event);
}

void CWinSystemWayland::OnSetCursor(wayland::pointer_t& pointer, std::uint32_t serial)
{
  if (m_osCursorVisible)
  {
    LoadDefaultCursor();
    if (m_cursorSurface) // Cursor loading could have failed
    {
      pointer.set_cursor(serial, m_cursorSurface, m_cursorImage.hotspot_x(), m_cursorImage.hotspot_y());
    }
  }
  else
  {
    pointer.set_cursor(serial, wayland::surface_t(), 0, 0);
  }
}
