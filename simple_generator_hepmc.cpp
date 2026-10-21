#include "simple_generator.hpp"
#include "my_geometry.hpp"
#include "mctruth.hpp"
#include "mcparticle.hpp"

#include <numbers>
#include <cmath>

#include <HepMC3/FourVector.h>
#include <HepMC3/GenParticle.h>
#include <HepMC3/GenVertex.h>

#include "phlex/configuration.hpp"
#include "phlex/model/data_cell_index.hpp"
#include "phlex/source.hpp"


namespace examples {


    HepMC3::GenEvent generate_hepmc(/*geometry const& geom, */int i) {

    //momentum unit, length unit
    HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::CM);
    evt.set_event_number(i);

    auto vtx = std::make_shared<HepMC3::GenVertex>(
          HepMC3::FourVector(0., 0., 0., 0.)); // x,y,z,t -- at the origin at trigger time

    //Assume a 1 GeV neutrino (travelling only in z) strikes an at-rest neutron
    // and produces a muon at a fixed angle: nu + n -> mu + p.
    // Energy-momentum conservation fixes the muon energy once the angle is chosen.
    constexpr double enu = 1.0; // GeV
    constexpr double mn = 0.939565;
    constexpr double mp = 0.938272;
    constexpr double mmu = 0.105658;
    double const thetamu = 100 * std::numbers::pi / 180.;

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

    //enu == pnu == p_proton + p_muon
    double const pp_z = enu - pmu_z;

    //Creating data products
    auto muon = std::make_shared<HepMC3::GenParticle>(
        HepMC3::FourVector(0., pmu_T, pmu_z, emu),
        13, 1
    );
    muon->set_generated_mass(mmu);
    vtx->add_particle_out(muon);

    //Proton balances out pmu_T (transverse momentum)
    auto proton = std::make_shared<HepMC3::GenParticle>(
        HepMC3::FourVector(0., -pmu_T, pp_z, sqrt(pp_z*pp_z + pmu_T*pmu_T + mp*mp)),
        2212, 1
    );
    proton->set_generated_mass(mp);
    vtx->add_particle_out(proton);


    evt.add_vertex(vtx);
    return evt;
  }
} // namespace examples

PHLEX_REGISTER_PROVIDERS(m, config)
{
    auto const layer = config.get<std::string>("layer");
    auto const suffix = config.get<std::string>("output_suffix", std::string{"genevent"});
    using namespace phlex;

    m.provide("generate",
              [](data_cell_index const& id) -> HepMC3::GenEvent {
                  return examples::generate_hepmc(static_cast<std::size_t>(id.number()));
              })
      .output_product("input", experimental::identifier{suffix}, experimental::identifier{layer});
}