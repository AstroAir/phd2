//
//  guide_algorithm_trimmed_mean.h
//  PHD2 Guiding
//
//  Created by Edgar Klenske.
//  Copyright 2015-2016, Max Planck Society.

/*
 *  This source code is distributed under the following "BSD" license
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *    Neither the name of Bret McKee, Dad Dog Development, nor the names of its
 *     Craig Stark, Stark Labs nor the names of its
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
 */

#ifndef GUIDE_ALGORITHM_TRIMMED_MEAN_H
#define GUIDE_ALGORITHM_TRIMMED_MEAN_H

#define MW_DEBUG_FILE_ 1

#define MW_BUFFER_SIZE 256

#include <Eigen/Dense>
#include "guide_algorithm.h"

class wxStopWatch;

class GuideAlgorithmTrimmedMean : public GuideAlgorithm
{
private:
    struct mw_guide_parameters;
    class GuideAlgorithmTrimmedMeanDialogPane;

    mw_guide_parameters* parameters;

    void HandleTimestamps();
    void HandleMeasurements(double input);
    void HandleControls(double control_input);
    void StoreControls(double control_input);
    double PredictDriftError();

protected:

    double GetControlGain() const;
    double GetPredictionGain() const;
    double GetDifferentialGain() const;
    bool SetControlGain(double control_gain);
    bool SetPredictionGain(double prediction_gain);
    bool SetDifferentialGain(double differential_gain);

    // minimum amount of points for starting the inference
    bool SetNbElementForInference(int nb_elements);
    int GetNbMeasurementsMin() const;

    bool GetDarkTracking();
    bool SetDarkTracking(bool value);

public:
    GuideAlgorithmTrimmedMean(Mount *pMount, GuideAxis axis);
    virtual ~GuideAlgorithmTrimmedMean(void);
    virtual GUIDE_ALGORITHM Algorithm(void);

    virtual ConfigDialogPane *GetConfigDialogPane(wxWindow *pParent);
    virtual double result(double input);
    virtual double deduceResult(void);
    virtual void reset();
    virtual wxString GetSettingsSummary();
    virtual wxString GetGuideAlgorithmClassName(void) const { return "Trimmed Mean"; }

};

#endif  // GUIDE_ALGORITHM_TRIMMED_MEAN_H
