/*
 *  advanced_dialog.h
 *  PHD Guiding
 *
 *  Created by Craig Stark.
 *  Copyright (c) 2006-2010 Craig Stark.
 *  All rights reserved.
 *
 *  This source code is distributed under the following "BSD" license
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *    Neither the name of Craig Stark, Stark Labs nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ADVANCED_DIALOG_H_INCLUDED
#define ADVANCED_DIALOG_H_INCLUDED

class MyFrame;
class MyFrameConfigDialogPane;
class MyFrameConfigDialogCtrlSet;
class MountConfigDialogCtrlSet;
class CameraConfigDialogPane;
class GuiderConfigDialogPane;

enum TAB_PAGES {
    AD_GLOBAL_PAGE,
    AD_GUIDER_PAGE,
    AD_CAMERA_PAGE,
    AD_MOUNT_PAGE,
    AD_AO_PAGE,
    AD_ROTATOR_PAGE,
    AD_UNASSIGNED_PAGE
};
// Segmented by the tab page location seen in the UI
enum BRAIN_CTRL_IDS : unsigned int
{
    AD_UNASSIGNED,
    AD_cbResetConfig,
    AD_cbDontAsk,
    AD_szImageLoggingFormat,
    AD_szLanguage,
    AD_szLogFileInfo,
    AD_szDitherRAOnly,
    AD_szDitherScale,
    AD_GLOBAL_TAB_BOUNDARY,        //-----end of global tab controls
    AD_cbUseSubFrames,
    AD_szNoiseReduction,
    AD_szAutoExposure,
    AD_szCameraTimeout,
    AD_szTimeLapse,
    AD_szPixelSize,
    AD_szGain,
    AD_szDelay,
    AD_szPort,
    AD_CAMERA_TAB_BOUNDARY,        // ------ end of camera tab controls
    AD_cbScaleImages,
    AD_szFocalLength,
    AD_cbAutoRestoreCal,
    AD_cbFastRecenter,
    AD_szStarTracking,
    AD_cbClearCalibration,
    AD_cbEnableGuiding,
    AD_szCalibrationDuration,
    AD_cbReverseDecOnFlip,
    AD_cbAssumeOrthogonal,
    AD_cbSlewDetection,
    AD_cbUseDecComp,
    AD_GUIDER_TAB_BOUNDARY,        // --------------- end of guiding tab controls
    AD_cbDecComp,
    AD_szDecCompAmt,
    AD_szMaxRAAmt,
    AD_szMaxDecAmt,
    AD_szDecGuideMode,
    AD_MOUNT_TAB_BOUNDARY,          // ----------- end of mount tab controls
    AD_szCalStepsPerIteration,
    AD_szSamplesToAverage,
    AD_szBumpPercentage,
    AD_szBumpSteps,
    AD_cbBumpOnDither,
    AD_cbClearAOCalibration,
    AD_cbEnableAOGuiding,
    AD_cbRotatorReverse,
    AD_DEVICES_TAB_BOUNDARY         // ----------- end of devices tab controls

};

struct BrainCtrlInfo
{
    BRAIN_CTRL_IDS ctrlId;
    wxObject* panelCtrl;
    bool isPositioned;           // debug only

    BrainCtrlInfo()
    {
        panelCtrl = NULL;
        ctrlId = AD_UNASSIGNED;
        isPositioned = false;
    }
    BrainCtrlInfo(BRAIN_CTRL_IDS id, wxObject* ctrl)
    {
        panelCtrl = ctrl;
        ctrlId = id;
        isPositioned = false;
    }

};

class AdvancedDialog : public wxDialog
{
    MyFrame *m_pFrame;
    wxBookCtrlBase *m_pNotebook;
    wxWindow *m_aoPage;
    wxWindow *m_rotatorPage;
    MyFrameConfigDialogPane *m_pGlobalPane;
    Guider::GuiderConfigDialogPane* m_pGuiderPane;
    CameraConfigDialogPane *m_pCameraPane;
    Mount::MountConfigDialogPane *m_pMountPane;
    AOConfigDialogPane *m_pAOPane;
    RotatorConfigDialogPane *m_pRotatorPane;

    std::map <BRAIN_CTRL_IDS, BrainCtrlInfo> m_brainCtrls;
    bool m_rebuildPanels;
    MyFrameConfigDialogCtrlSet *m_pGlobalCtrlSet;
    CameraConfigDialogCtrlSet *m_pCameraCtrlSet;
    GuiderConfigDialogCtrlSet *m_pGuiderCtrlSet;
    MountConfigDialogCtrlSet *m_pScopeCtrlSet;
    AOConfigDialogCtrlSet *m_pAOCtrlSet;
    RotatorConfigDialogCtrlSet *m_pRotatorCtrlSet;
    wxPanel *m_pGlobalSettingsPanel;
    wxPanel *m_pCameraSettingsPanel;
    wxPanel *m_pGuiderSettingsPanel;
    wxPanel *m_pScopeSettingsPanel;
    wxPanel *m_pDevicesSettingsPanel;

public:
    AdvancedDialog(MyFrame *pFrame);
    ~AdvancedDialog();

    void EndModal(int retCode);

    void UpdateCameraPage(void);
    void UpdateMountPage(void);
    void UpdateAoPage(void);
    void UpdateRotatorPage(void);

    void LoadValues(void);
    void UnloadValues(void);
    void Undo(void);
    void Preload();

    int GetFocalLength(void);
    void SetFocalLength(int val);
    double GetPixelSize(void);
    void SetPixelSize(double val);

    wxWindow* GetTabLocation(BRAIN_CTRL_IDS id);


private:
    void AddCameraPage(void);
    void AddMountPage(void);
    void AddAoPage(void);
    void AddRotatorPage(void);
    void RebuildPanels(void);
    void BuildCtrlSets();
    void CleanupCtrlSets();
    void ConfirmLayouts();
};

#endif // ADVANCED_DIALOG_H_INCLUDED
