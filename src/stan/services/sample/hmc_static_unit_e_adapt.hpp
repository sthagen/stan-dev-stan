#ifndef STAN_SERVICES_SAMPLE_HMC_STATIC_UNIT_E_ADAPT_HPP
#define STAN_SERVICES_SAMPLE_HMC_STATIC_UNIT_E_ADAPT_HPP

#include <stan/callbacks/interrupt.hpp>
#include <stan/callbacks/logger.hpp>
#include <stan/callbacks/structured_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/io/var_context.hpp>
#include <stan/math/prim.hpp>
#include <stan/mcmc/hmc/static/adapt_unit_e_static_hmc.hpp>
#include <stan/services/error_codes.hpp>
#include <stan/services/util/create_rng.hpp>
#include <stan/services/util/initialize.hpp>
#include <stan/services/util/run_adaptive_sampler.hpp>
#include <iostream>
#include <vector>

namespace stan {
namespace services {
namespace sample {

/**
 * Runs static HMC with unit Euclidean
 * metric with adaptation.
 *
 * @tparam Model Model class
 * @param[in] model Input model to test (with data already instantiated)
 * @param[in] init var context for initialization
 * @param[in] random_seed random seed for the random number generator
 * @param[in] chain chain id to advance the pseudo random number generator
 * @param[in] init_radius radius to initialize
 * @param[in] num_warmup Number of warmup samples
 * @param[in] num_samples Number of samples
 * @param[in] num_thin Number to thin the samples
 * @param[in] save_warmup Indicates whether to save the warmup iterations
 * @param[in] refresh Controls the output
 * @param[in] stepsize initial stepsize for discrete evolution
 * @param[in] stepsize_jitter uniform random jitter of stepsize
 * @param[in] int_time integration time
 * @param[in] delta adaptation target acceptance statistic
 * @param[in] gamma adaptation regularization scale
 * @param[in] kappa adaptation relaxation exponent
 * @param[in] t0 adaptation iteration offset
 * @param[in,out] interrupt Callback for interrupts
 * @param[in,out] logger Logger for messages
 * @param[in,out] init_writer Writer callback for unconstrained inits
 * @param[in,out] sample_writer Writer for draws
 * @param[in,out] diagnostic_writer Writer for diagnostic information
 * @return error_codes::OK if successful
 */
template <class Model>
int hmc_static_unit_e_adapt(
    Model& model, const stan::io::var_context& init, unsigned int random_seed,
    unsigned int chain, double init_radius, int num_warmup, int num_samples,
    int num_thin, bool save_warmup, int refresh, double stepsize,
    double stepsize_jitter, double int_time, double delta, double gamma,
    double kappa, double t0, callbacks::interrupt& interrupt,
    callbacks::logger& logger, callbacks::writer& init_writer,
    callbacks::writer& sample_writer, callbacks::writer& diagnostic_writer) {
  boost::ecuyer1988 rng = util::create_rng(random_seed, chain);

  std::vector<int> disc_vector;
  std::vector<double> cont_vector;

  try {
    cont_vector = util::initialize(model, init, rng, init_radius, true, logger,
                                   init_writer);
  } catch (const std::exception& e) {
    logger.error(e.what());
    return error_codes::CONFIG;
  }
  stan::mcmc::adapt_unit_e_static_hmc<Model, boost::ecuyer1988> sampler(model,
                                                                        rng);
  sampler.set_nominal_stepsize_and_T(stepsize, int_time);
  sampler.set_stepsize_jitter(stepsize_jitter);

  sampler.get_stepsize_adaptation().set_mu(log(10 * stepsize));
  sampler.get_stepsize_adaptation().set_delta(delta);
  sampler.get_stepsize_adaptation().set_gamma(gamma);
  sampler.get_stepsize_adaptation().set_kappa(kappa);
  sampler.get_stepsize_adaptation().set_t0(t0);

  callbacks::structured_writer dummy_metric_writer;
  util::run_adaptive_sampler(sampler, model, cont_vector, num_warmup,
                             num_samples, num_thin, refresh, save_warmup, rng,
                             interrupt, logger, sample_writer,
                             diagnostic_writer, dummy_metric_writer);

  return error_codes::OK;
}

}  // namespace sample
}  // namespace services
}  // namespace stan
#endif
