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

#include "WinSystemWaylandEGLContextGLES.h"

#include <EGL/egl.h>

#include "cores/VideoPlayer/VideoRenderers/RenderFactory.h"
#include "cores/VideoPlayer/VideoRenderers/LinuxRendererGLES.h"
#include "utils/log.h"

#if defined(HAVE_LIBVA)
#include "cores/VideoPlayer/DVDCodecs/Video/VAAPI.h"
#include "cores/VideoPlayer/VideoRenderers/HwDecRender/RendererVAAPIGLES.h"
#endif

using namespace KODI::WINDOWING::WAYLAND;

bool CWinSystemWaylandEGLContextGLES::InitWindowSystem()
{
  if (!CWinSystemWaylandEGLContext::InitWindowSystemEGL(EGL_OPENGL_ES2_BIT, EGL_OPENGL_ES_API))
  {
    return false;
  }

  CLinuxRendererGLES::Register();

#if defined(HAVE_LIBVA)
  bool general, hevc;
  CRendererVAAPI::Register(GetVaDisplay(), m_eglContext.m_eglDisplay, general, hevc);
  if (general)
  {
    VAAPI::CDecoder::Register(hevc);
  }
#endif

  return true;
}

bool CWinSystemWaylandEGLContextGLES::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
  if (!CWinSystemWaylandEGLContext::SetFullScreen(fullScreen, res, blankOtherDisplays))
  {
    return false;
  }

  // Propagate changed dimensions to render system if necessary
  if (m_nWidth != CRenderSystemGLES::m_width || m_nHeight != CRenderSystemGLES::m_height)
  {
    CLog::LogF(LOGDEBUG, "Resetting render system to %dx%d", m_nWidth, m_nHeight);
    if (!CRenderSystemGLES::ResetRenderSystem(m_nWidth, m_nHeight, fullScreen, res.fRefreshRate))
    {
      return false;
    }
  }

  return true;
}

void CWinSystemWaylandEGLContextGLES::SetVSyncImpl(bool enable)
{
  m_eglContext.SetVSync(enable);
}

void CWinSystemWaylandEGLContextGLES::PresentRenderImpl(bool rendered)
{
  PresentFrame(rendered);
}
