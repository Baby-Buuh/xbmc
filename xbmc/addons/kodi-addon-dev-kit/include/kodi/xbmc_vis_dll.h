#pragma once

/*
 *      Copyright (C) 2005-2015 Team Kodi
 *      http://kodi.tv
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "xbmc_addon_dll.h"
#include "xbmc_vis_types.h"

extern "C"
{
  // Functions that your visualisation must implement
  void Start(int iChannels, int iSamplesPerSec, int iBitsPerSample, const char* szSongName);
  void Stop();
  void AudioData(const float* pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength);
  void Render();
  bool OnAction(long action, const void *param);
  void GetInfo(VIS_INFO* pInfo);
  unsigned int GetPresets(char ***presets);
  unsigned GetPreset();
  unsigned int GetSubModules(char ***presets);
  bool IsLocked();

  // function to export the above structure to XBMC
  void __declspec(dllexport) get_addon(void* ptr)
  {
    AddonInstance_Visualization* pVisz = static_cast<AddonInstance_Visualization*>(ptr);

    pVisz->toAddon.Start = Start;
    pVisz->toAddon.Stop = Stop;
    pVisz->toAddon.AudioData = AudioData;
    pVisz->toAddon.Render = Render;
    pVisz->toAddon.OnAction = OnAction;
    pVisz->toAddon.GetInfo = GetInfo;
    pVisz->toAddon.GetPresets = GetPresets;
    pVisz->toAddon.GetPreset = GetPreset;
    pVisz->toAddon.GetSubModules = GetSubModules;
    pVisz->toAddon.IsLocked = IsLocked;
  };
};

