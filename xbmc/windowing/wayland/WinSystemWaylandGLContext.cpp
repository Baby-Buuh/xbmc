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

#include "WinSystemWaylandGLContext.h"

using namespace KODI::WINDOWING::WAYLAND;


#if defined(HAS_GL)

bool CWinSystemWaylandGLContext::CreateNewWindow(const std::string& name,
                                               bool fullScreen,
                                               RESOLUTION_INFO& res,
                                               PHANDLE_EVENT_FUNC userFunction)
{
  return SetFullScreen(fullScreen, res, false);
}

bool CWinSystemWaylandGLContext::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
  auto ret = CWinSystemWayland::SetFullScreen(fullScreen, res, blankOtherDisplays);
  if (ret)
  {
    CRenderSystemGL::ResetRenderSystem(res.iWidth, res.iHeight, fullScreen, 0);
  }

  return ret;
}

void CWinSystemWaylandGLContext::SetVSyncImpl(bool enable)
{
  m_glContext.SetVSync(enable);
}

void CWinSystemWaylandGLContext::PresentRenderImpl(bool rendered)
{
  if (rendered)
  {
    m_glContext.SwapBuffers();
  }
}

EGLDisplay CWinSystemWaylandGLContext::GetEGLDisplay() const
{
  return m_glContext.m_eglDisplay;
}

EGLSurface CWinSystemWaylandGLContext::GetEGLSurface() const
{
  return m_glContext.m_eglSurface;
}

EGLContext CWinSystemWaylandGLContext::GetEGLContext() const
{
  return m_glContext.m_eglContext;
}

EGLConfig  CWinSystemWaylandGLContext::GetEGLConfig() const
{
  return m_glContext.m_eglConfig;
}

bool CWinSystemWaylandGLContext::IsExtSupported(const char* extension)
{
  return m_glContext.IsExtSupported(extension);
}

#endif
