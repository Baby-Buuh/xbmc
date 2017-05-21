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

#include "GLContextEGL.h"
#include "rendering/gl/RenderSystemGL.h"
#include "utils/GlobalsHandling.h"
#include "WinSystemWayland.h"

namespace KODI
{
namespace WINDOWING
{
namespace WAYLAND
{

#if defined(HAS_GL)
class CWinSystemWaylandGLContext : public CWinSystemWayland, public CRenderSystemGL
{
public:
  CWinSystemWaylandGLContext() = default;
  virtual ~CWinSystemWaylandGLContext() = default;

  bool CreateNewWindow(const std::string& name,
                       bool fullScreen,
                       RESOLUTION_INFO& res,
                       PHANDLE_EVENT_FUNC userFunction) override;

  bool SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays) override;
  bool IsExtSupported(const char* extension) override;
  EGLDisplay GetEGLDisplay() const;
  EGLSurface GetEGLSurface() const;
  EGLContext GetEGLContext() const;
  EGLConfig  GetEGLConfig() const;

protected:
  void SetVSyncImpl(bool enable) override;
  void PresentRenderImpl(bool rendered) override;

private:
  CGLContextEGL m_glContext;
};
#endif

}
}
}

#if defined(HAS_GL)
XBMC_GLOBAL_REF(KODI::WINDOWING::WAYLAND::CWinSystemWaylandGLContext, g_Windowing);
#define g_Windowing XBMC_GLOBAL_USE(KODI::WINDOWING::WAYLAND::CWinSystemWaylandGLContext)
#endif