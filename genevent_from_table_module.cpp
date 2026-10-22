#include "phlex/module.hpp"

#include <HepMC3/FourVector.h>
#include <HepMC3/GenEvent.h>
#include <HepMC3/GenParticle.h>
#include <HepMC3/GenVertex.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
  // Particle table produced by py_simple_generator_hepmc.py (float64, GeV and cm):
  //   [event_number, x, y, z, t,  then per particle: pdg, status, px, py, pz, E, mass]
  constexpr std::size_t header_size = 5;
  constexpr std::size_t particle_size = 7;

  HepMC3::GenEvent table_to_genevent(std::shared_ptr<std::vector<double>> const& table)
  {
    if (!table) {
      throw std::runtime_error("genevent_from_table: null particle table");
    }
    auto const& t = *table;
    if (t.size() < header_size || (t.size() - header_size) % particle_size != 0) {
      throw std::runtime_error("genevent_from_table: table length " + std::to_string(t.size()) +
                               " is not 5 + 7*n");
    }

    HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::CM);
    evt.set_event_number(static_cast<int>(t[0]));
    auto vtx = std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector(t[1], t[2], t[3], t[4]));
    for (std::size_t i = header_size; i < t.size(); i += particle_size) {
      auto particle = std::make_shared<HepMC3::GenParticle>(
        HepMC3::FourVector(t[i + 2], t[i + 3], t[i + 4], t[i + 5]),
        static_cast<int>(t[i]),
        static_cast<int>(t[i + 1]));
      particle->set_generated_mass(t[i + 6]);
      vtx->add_particle_out(particle);
    }
    evt.add_vertex(vtx);
    return evt;
  }
}

PHLEX_REGISTER_ALGORITHMS(m, config)
{
  using namespace phlex;

  // Named "input" by default so the output matches what the C++ sources provide
  // (creator "input", suffix "genevent").
  auto const name = config.get<std::string>("name", std::string{"input"});
  auto const input_creator = config.get<std::string>("input_creator", std::string{"pygen"});
  auto const input_layer = config.get<std::string>("input_layer");
  auto const input_suffix = config.get<std::string>("input_suffix", std::string{"particles"});
  auto const output_suffix = config.get<std::string>("output_suffix", std::string{"genevent"});

  m.transform(name, table_to_genevent, concurrency::unlimited)
    .input_family(product_selector{.creator = input_creator,
                                   .layer = input_layer,
                                   .suffix = experimental::identifier{input_suffix}})
    .output_product_suffixes(output_suffix);
}
