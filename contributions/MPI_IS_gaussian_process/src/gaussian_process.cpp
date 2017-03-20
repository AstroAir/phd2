/*
 * Copyright 2014-2015, Max Planck Society.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*!@file
 * @author  Edgar Klenske <edgar.klenske@tuebingen.mpg.de>
 * @author  Stephan Wenninger <stephan.wenninger@tuebingen.mpg.de>
 *
 */

#include <cstdint>

#include "gaussian_process.h"
#include "math_tools.h"
#include "covariance_functions.h"

GP::GP() : covFunc_(0), // initialize pointer to null
    data_loc_(Eigen::VectorXd()),
    data_out_(Eigen::VectorXd()),
    gram_matrix_(Eigen::MatrixXd()),
    alpha_(Eigen::VectorXd()),
    chol_gram_matrix_(Eigen::LDLT<Eigen::MatrixXd>()),
    log_noise_sd_(-1E20),
    use_explicit_trend_(false),
    feature_vectors_(Eigen::MatrixXd()),
    feature_matrix_(Eigen::MatrixXd()),
    chol_feature_matrix_(Eigen::LDLT<Eigen::MatrixXd>()),
    beta_(Eigen::VectorXd())
{ }

GP::GP(const covariance_functions::CovFunc& covFunc) :
    covFunc_(covFunc.clone()),
    data_loc_(Eigen::VectorXd()),
    data_out_(Eigen::VectorXd()),
    gram_matrix_(Eigen::MatrixXd()),
    alpha_(Eigen::VectorXd()),
    chol_gram_matrix_(Eigen::LDLT<Eigen::MatrixXd>()),
    log_noise_sd_(-1E20),
    use_explicit_trend_(false),
    feature_vectors_(Eigen::MatrixXd()),
    feature_matrix_(Eigen::MatrixXd()),
    chol_feature_matrix_(Eigen::LDLT<Eigen::MatrixXd>()),
    beta_(Eigen::VectorXd())
{ }

GP::GP(const double noise_variance,
       const covariance_functions::CovFunc& covFunc) :
    covFunc_(covFunc.clone()),
    data_loc_(Eigen::VectorXd()),
    data_out_(Eigen::VectorXd()),
    gram_matrix_(Eigen::MatrixXd()),
    alpha_(Eigen::VectorXd()),
    chol_gram_matrix_(Eigen::LDLT<Eigen::MatrixXd>()),
    log_noise_sd_(std::log(noise_variance)),
    use_explicit_trend_(false),
    feature_vectors_(Eigen::MatrixXd()),
    feature_matrix_(Eigen::MatrixXd()),
    chol_feature_matrix_(Eigen::LDLT<Eigen::MatrixXd>()),
    beta_(Eigen::VectorXd())
{ }

GP::~GP()
{
    delete this->covFunc_; // tidy up since we are responsible for the covFunc.
}

GP::GP(const GP& that) :
    covFunc_(0),
    data_loc_(that.data_loc_),
    data_out_(that.data_out_),
    gram_matrix_(that.gram_matrix_),
    gram_matrix_derivatives_(that.gram_matrix_derivatives_),
    alpha_(that.alpha_),
    chol_gram_matrix_(that.chol_gram_matrix_),
    log_noise_sd_(that.log_noise_sd_),
    use_explicit_trend_(that.use_explicit_trend_),
    feature_vectors_(that.feature_vectors_),
    feature_matrix_(that.feature_matrix_),
    chol_feature_matrix_(that.chol_feature_matrix_),
    beta_(that.beta_)
{
    covFunc_ = that.covFunc_->clone();
}

bool GP::setCovarianceFunction(const covariance_functions::CovFunc& covFunc)
{
    if (data_loc_.size() != 0 || data_out_.size() != 0)
        return false;
    delete covFunc_;
    covFunc_ = covFunc.clone();

    return true;
}

GP& GP::operator=(const GP& that)
{
    if (this != &that)
    {
        covariance_functions::CovFunc* temp = covFunc_;  // store old pointer...
        covFunc_ = that.covFunc_->clone();  // ... first clone ...
        delete temp;  // ... and then delete.

        // copy the rest
        data_loc_ = that.data_loc_;
        data_out_ = that.data_out_;
        gram_matrix_ = that.gram_matrix_;
        gram_matrix_derivatives_ = that.gram_matrix_derivatives_;
        alpha_ = that.alpha_;
        chol_gram_matrix_ = that.chol_gram_matrix_;
        log_noise_sd_ = that.log_noise_sd_;
    }
    return *this;
}

Eigen::VectorXd GP::drawSample(const Eigen::VectorXd& locations) const
{
    return drawSample(locations,
                      math_tools::generate_normal_random_matrix(locations.rows(), locations.cols()));
}

Eigen::VectorXd GP::drawSample(const Eigen::VectorXd& locations,
                               const Eigen::VectorXd& random_vector) const
{
    Eigen::MatrixXd prior_covariance;
    Eigen::MatrixXd kernel_matrix;
    Eigen::LLT<Eigen::MatrixXd> chol_kernel_matrix;
    Eigen::MatrixXd samples;

    // we need the prior covariance for both, prior and posterior samples.
    prior_covariance = covFunc_->evaluate(locations, locations).first;
    kernel_matrix = prior_covariance;

    if (gram_matrix_.cols() == 0)   // i.e. only a prior
    {
        kernel_matrix = prior_covariance + JITTER * Eigen::MatrixXd::Identity(
                            prior_covariance.rows(), prior_covariance.cols());
    }
    else
    {
        Eigen::MatrixXd mixed_covariance;
        mixed_covariance = covFunc_->evaluate(locations, data_loc_).first;
        Eigen::MatrixXd posterior_covariance;
        posterior_covariance = prior_covariance - mixed_covariance *
                               (chol_gram_matrix_.solve(mixed_covariance.transpose()));
        kernel_matrix = posterior_covariance + JITTER * Eigen::MatrixXd::Identity(
                            posterior_covariance.rows(), posterior_covariance.cols());
    }
    chol_kernel_matrix = kernel_matrix.llt();

    // Draw sample: s = chol(K)*x, where x is a random vector
    samples = chol_kernel_matrix.matrixL() * random_vector;

    return samples + std::exp(log_noise_sd_) *
           math_tools::generate_normal_random_matrix(samples.rows(), samples.cols());
}

void GP::infer()
{
    assert(data_loc_.rows() > 0 && "Error: the GP is not yet initialized!");

    // The data covariance matrix
    covariance_functions::MatrixStdVecPair cov_result =
        covFunc_->evaluate(data_loc_, data_loc_);
    Eigen::MatrixXd& data_cov = cov_result.first;

    gram_matrix_derivatives_.resize(cov_result.second.size() + 1);
    for (size_t i = 0; i < cov_result.second.size(); i++)
    {
        gram_matrix_derivatives_[i + 1].swap(cov_result.second[i]);
    }
    // noise derivative first
    gram_matrix_derivatives_[0] = 2 * std::exp(2 * log_noise_sd_) *
                                  Eigen::MatrixXd::Identity(data_cov.rows(), data_cov.cols());

    // compute and store the Gram matrix
    gram_matrix_.swap(data_cov);
    gram_matrix_ += (std::exp(2 * log_noise_sd_) + JITTER) *
                    Eigen::MatrixXd::Identity(gram_matrix_.rows(), gram_matrix_.cols());

    // compute the Cholesky decomposition of the Gram matrix
    chol_gram_matrix_ = gram_matrix_.ldlt();

    // pre-compute the alpha, which is the solution of the chol to the data.
    alpha_ = chol_gram_matrix_.solve(data_out_);

    if (use_explicit_trend_)
    {
        feature_vectors_ = Eigen::MatrixXd(2, data_loc_.rows());
        // precompute necessary matrices for the explicit trend function
        feature_vectors_.row(0) = data_loc_.array().pow(0);
        feature_vectors_.row(1) = data_loc_.array().pow(1);

        feature_matrix_ = feature_vectors_ * chol_gram_matrix_.solve(feature_vectors_.transpose());
        chol_feature_matrix_ = feature_matrix_.ldlt();

        beta_ = chol_feature_matrix_.solve(feature_vectors_) * alpha_;
    }
}

void GP::infer(const Eigen::VectorXd& data_loc,
               const Eigen::VectorXd& data_out)
{
    data_loc_ = data_loc;
    data_out_ = data_out;
    infer(); // updates the Gram matrix and its Cholesky decomposition
}

void GP::clear()
{
    gram_matrix_ = Eigen::MatrixXd();
    chol_gram_matrix_ = Eigen::LDLT<Eigen::MatrixXd>();
    data_loc_ = Eigen::VectorXd();
    data_out_ = Eigen::VectorXd();
}

GP::VectorMatrixPair GP::predict(const Eigen::VectorXd& locations) const
{

    // The prior covariance matrix (evaluated on test points)
    Eigen::MatrixXd prior_cov = this->covFunc_->evaluate(
                                    locations, locations).first;

    if (data_loc_.rows() == 0)  // check if the data is empty
    {
        return std::make_pair(Eigen::VectorXd::Zero(locations.size()), prior_cov);
    }
    else
    {

        // The mixed covariance matrix (test and data points)
        Eigen::MatrixXd mixed_cov = this->covFunc_->evaluate(
                                        locations, data_loc_).first;

        Eigen::MatrixXd phi(2, locations.rows());
        if (use_explicit_trend_)
        {
            phi.row(0) = locations.array().pow(0);
            phi.row(1) = locations.array().pow(1);

            return predict(prior_cov, mixed_cov, phi);
        }

        return predict(prior_cov, mixed_cov);
    }
}

GP::VectorMatrixPair GP::predict(const Eigen::MatrixXd& prior_cov,
                                 const Eigen::MatrixXd& mixed_cov,
                                 const Eigen::MatrixXd& phi) const
{

    Eigen::VectorXd m = mixed_cov * alpha_;

    Eigen::MatrixXd v = prior_cov - mixed_cov *
                        (chol_gram_matrix_.solve(mixed_cov.transpose()));

    if (use_explicit_trend_)
    {
        Eigen::MatrixXd R = phi - feature_vectors_ * chol_gram_matrix_.solve(mixed_cov.transpose());
        Eigen::MatrixXd B = R.transpose() * chol_feature_matrix_.solve(R);

        m += R.transpose() * beta_;
        v += B;
    }

    return std::make_pair(m, v);
}

double GP::neg_log_likelihood() const
{
    double result = 0;
    if (gram_matrix_.rows() > 0)
    {
        // Implmented according to Equation (5.8) in Rasmussen & Williams, 2006
        Eigen::MatrixXd Z = chol_gram_matrix_.matrixL();
        result = data_out_.transpose() * chol_gram_matrix_.solve(data_out_);
        result += chol_gram_matrix_.vectorD().array().log().sum();
        result += data_out_.rows() * std::log(2 * M_PI);
    }
    return 0.5 * result;
}

Eigen::VectorXd GP::neg_log_likelihood_gradient() const
{
    Eigen::VectorXd result;
    result = Eigen::VectorXd(gram_matrix_derivatives_.size());

    Eigen::MatrixXd beta(gram_matrix_.rows(), gram_matrix_.cols());
    // Implmented according to Equation (5.9) in Rasmussen & Williams, 2006
    for (size_t i = 0; i < gram_matrix_derivatives_.size(); ++i)
    {
        beta = chol_gram_matrix_.solve(gram_matrix_derivatives_[i]);
        result[i] = -0.5 * (alpha_.transpose() * gram_matrix_derivatives_[i] * alpha_ -
                            beta.trace());
    }
    return result;
}

void GP::setHyperParameters(const Eigen::VectorXd& hyperParameters)
{
    assert(hyperParameters.rows() == covFunc_->getParameterCount() + covFunc_->getExtraParameterCount() + 1 &&
           "Wrong number of hyperparameters supplied to setHyperParameters()!");
    log_noise_sd_ = hyperParameters[0];
    covFunc_->setParameters(hyperParameters.segment(1, covFunc_->getParameterCount()));
    covFunc_->setExtraParameters(hyperParameters.tail(covFunc_->getExtraParameterCount()));
    if (data_loc_.rows() > 0)
    {
        infer();
    }
}

Eigen::VectorXd GP::getHyperParameters() const
{
    Eigen::VectorXd hyperParameters(covFunc_->getParameterCount() + covFunc_->getExtraParameterCount() + 1);
    hyperParameters << log_noise_sd_, covFunc_->getParameters(), covFunc_->getExtraParameters();
    return hyperParameters;
}

void GP::setCovarianceHyperParameters(const Eigen::VectorXd& hyperParameters)
{
    assert(hyperParameters.rows() == covFunc_->getParameterCount() &&
           "Wrong number of hyperparameters supplied to"
           "setCovarianceHyperParameters()!");
    covFunc_->setParameters(hyperParameters);
    infer();
}

void GP::enableExplicitTrend()
{
    use_explicit_trend_ = true;
}

void GP::disableExplicitTrend()
{
    use_explicit_trend_ = false;
}
