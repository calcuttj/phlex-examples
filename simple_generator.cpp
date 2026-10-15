#include "simple_generator.hpp"
#include "my_geometry.hpp"
#include "mctruth.hpp"
#include "mcparticle.hpp"
#include "phlex/module.hpp"

#include <numbers>
#include <cmath>



namespace examples {

  MCTruth generate(geometry const& geom, int i) {
    examples::MCTruth output("simple_generator");

    examples::MCParticle muon(
      13, 0, 0, -1,
      "generator"
    );

    //Assume a 1 GeV neutrino (travelling only in z) strikes an at-rest neutron
    // and produces a muon at a fixed angle: nu + n -> mu + p.
    // Energy-momentum conservation fixes the muon energy once the angle is chosen.
    constexpr double enu = 1.0; // GeV
    constexpr double mn = 0.939565;
    constexpr double mp = 0.938272;
    constexpr double mmu = 0.105658;
    double const thetamu = 45.0 * std::numbers::pi / 180.;

    // Eliminating the proton four-momentum gives A*pmu - B*Emu + C = 0 with
    // pmu = sqrt(Emu^2 - mmu^2). Squaring yields a quadratic in Emu.
    double const A = 2. * enu * std::cos(thetamu);
    double const B = 2. * (enu + mn);
    double const C = mn * mn - mp * mp + mmu * mmu + 2. * enu * mn;
    double const D = B * B - A * A;
    // The "+" root is physical; the "-" root fails B*Emu - C >= 0 (spurious from squaring).
    double const emu = (B * C + A * std::sqrt(C * C - mmu * mmu * D)) / D;

    double const pmu = std::sqrt(emu * emu - mmu * mmu);
    double const pmu_z = std::cos(thetamu) * pmu;
    double const pmu_T = std::sin(thetamu) * pmu;
    muon.AddTrajectoryPoint({
      0, 0, 0, 0, //Position, time
      0, pmu_T, pmu_z // momentum
    });
    output.Add(muon);
    double const pp_z = enu - pmu_z;
    examples::MCParticle proton(
      2212, 0, 1, -1,
      "generator"
    );
    proton.AddTrajectoryPoint({
      0, 0, 0, 0,
      0, -pmu_T, pp_z
    });
    output.Add(proton);

    return output;
  }
} // namespace examples


PHLEX_REGISTER_ALGORITHMS(m, config)
{
  using namespace phlex;

  auto const layer = config.get<std::string>("layer");

  m.transform("generate", examples::generate, concurrency::unlimited)
    .input_family(product_selector{.creator = "input", .layer = "job"},
                  product_selector{.creator = "input", .layer = layer, .suffix = "i"})
    .output_product_suffixes("generator_output");

}
