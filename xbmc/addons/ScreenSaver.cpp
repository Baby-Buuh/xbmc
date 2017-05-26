/*
 *      Copyright (C) 2005-2013 Team XBMC
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
#include "ScreenSaver.h"
#include "ServiceBroker.h"
#include "filesystem/SpecialProtocol.h"
#include "guilib/GraphicContext.h"
#include "interfaces/generic/ScriptInvocationManager.h"
#include "settings/Settings.h"
#include "utils/AlarmClock.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "windowing/WindowingFactory.h"

// What sound does a python screensaver make?
#define SCRIPT_ALARM "sssssscreensaver"

#define SCRIPT_TIMEOUT 15 // seconds

namespace ADDON
{

CScreenSaver::CScreenSaver(AddonProps props)
 : ADDON::CAddonDll(std::move(props))
{
  m_struct = {0};
}

CScreenSaver::CScreenSaver(const char *addonID)
 : ADDON::CAddonDll(AddonProps(addonID, ADDON_UNKNOWN))
{
  m_struct = {0};
}

bool CScreenSaver::IsInUse() const
{
  return CServiceBroker::GetSettings().GetString(CSettings::SETTING_SCREENSAVER_MODE) == ID();
}

bool CScreenSaver::CreateScreenSaver()
{
  if (CScriptInvocationManager::GetInstance().HasLanguageInvoker(LibPath()))
  {
    // Don't allow a previously-scheduled alarm to kill our new screensaver
    g_alarmClock.Stop(SCRIPT_ALARM, true);

    if (!CScriptInvocationManager::GetInstance().Stop(LibPath()))
      CScriptInvocationManager::GetInstance().ExecuteAsync(LibPath(), AddonPtr(new CScreenSaver(*this)));
    return true;
  }

  m_name = Name();
  m_presets = CSpecialProtocol::TranslatePath(Path());
  m_profile = CSpecialProtocol::TranslatePath(Profile());

#ifdef HAS_DX
  m_struct.props.device = g_Windowing.Get3D11Context();
#else
  m_struct.props.device = nullptr;
#endif
  m_struct.props.x = 0;
  m_struct.props.y = 0;
  m_struct.props.width = g_graphicsContext.GetWidth();
  m_struct.props.height = g_graphicsContext.GetHeight();
  m_struct.props.pixelRatio = g_graphicsContext.GetResInfo().fPixelRatio;
  m_struct.props.name = m_name.c_str();
  m_struct.props.presets = m_presets.c_str();
  m_struct.props.profile = m_profile.c_str();

  m_struct.toKodi.kodiInstance = this;

  /* Open the class "kodi::addon::CInstanceScreensaver" on add-on side */
  ADDON_STATUS status = CAddonDll::CreateInstance(ADDON_INSTANCE_SCREENSAVER, ID(), &m_struct);
  if (status != ADDON_STATUS_OK)
  if (!CAddonDll::CreateInstance(ADDON_INSTANCE_SCREENSAVER, ID(), &m_struct))
  {
    CLog::Log(LOGFATAL, "Screensaver: failed to create instance for '%s' and not usable!", ID().c_str());
    return false;
  }

  return true;
}

void CScreenSaver::Start()
{
  // notify screen saver that they should start
  if (m_struct.toAddon.Start)
    m_struct.toAddon.Start(&m_struct);
}

void CScreenSaver::Stop()
{
  if (m_struct.toAddon.Stop)
    m_struct.toAddon.Stop(&m_struct);
}

void CScreenSaver::Render()
{
  // ask screensaver to render itself
  if (m_struct.toAddon.Render)
    m_struct.toAddon.Render(&m_struct);
}

void CScreenSaver::Destroy()
{
  if (URIUtils::HasExtension(LibPath(), ".py"))
  {
    /* FIXME: This is a hack but a proper fix is non-trivial. Basically this code
     * makes sure the addon gets terminated after we've moved out of the screensaver window.
     * If we don't do this, we may simply lockup.
     */
    g_alarmClock.Start(SCRIPT_ALARM, SCRIPT_TIMEOUT, "StopScript(" + LibPath() + ")", true, false);
    return;
  }

  m_struct = {0};
  CAddonDll::DestroyInstance(ID());
}

} /* namespace ADDON */
