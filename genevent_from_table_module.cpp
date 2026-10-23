#include "phlex/module.hpp"

#include <HepMC3/FourVector.h>
#include <HepMC3/GenEvent.h>
#include <HepMC3/GenParticle.h>
#include <HepMC3/GenVertex.h>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
  // Event table produced by genevent_table.py (float64, GeV and cm). Each particle is
  // outgoing from one vertex, given by its index (from 0) into the vertex list:
  //   [event_number, n_vertices,
  //    then per vertex: x, y, z, t,
  //    then per particle: vertex, pdg, status, px, py, pz, E, mass]
  constexpr std::size_t header_size = 2;
  constexpr std::size_t vertex_size = 4;
  constexpr std::size_t particle_size = 8;

  HepMC3::GenEvent table_to_genevent(std::shared_ptr<std::vector<double>> const& table)
  {
    auto fail = [](std::string const& why) {
      throw std::runtime_error("genevent_from_table: " + why);
    };
    if (!table) {
      fail("null event table");
    }
    auto const& t = *table;
    if (t.size() < header_size) {
      fail("table length " + std::to_string(t.size()) + " is shorter than the header");
    }
    // Range checks are written so that NaN fails them, before any cast to an integer.
    auto const max_vertices = static_cast<double>((t.size() - header_size) / vertex_size);
    if (!(t[1] >= 0 && t[1] <= max_vertices) || t[1] != std::floor(t[1])) {
      fail("invalid vertex count " + std::to_string(t[1]));
    }
    auto const n_vertices = static_cast<std::size_t>(t[1]);
    auto const first_particle = header_size + vertex_size * n_vertices;
    if (t.size() < first_particle || (t.size() - first_particle) % particle_size != 0) {
      fail("table length " + std::to_string(t.size()) + " is not 2 + 4*" +
           std::to_string(n_vertices) + " + 8*n_particles");
    }

    HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::CM);
    evt.set_event_number(static_cast<int>(t[0]));

    std::vector<HepMC3::GenVertexPtr> vertices;
    vertices.reserve(n_vertices);
    for (std::size_t i = header_size; i < first_particle; i += vertex_size) {
      vertices.push_back(std::make_shared<HepMC3::GenVertex>(
        HepMC3::FourVector(t[i], t[i + 1], t[i + 2], t[i + 3])));
    }

    for (std::size_t i = first_particle; i < t.size(); i += particle_size) {
      auto const vertex = t[i];
      if (!(vertex >= 0 && vertex < static_cast<double>(n_vertices)) ||
          vertex != std::floor(vertex)) {
        fail("particle " + std::to_string((i - first_particle) / particle_size) +
             " has invalid vertex index " + std::to_string(vertex));
      }
      auto particle = std::make_shared<HepMC3::GenParticle>(
        HepMC3::FourVector(t[i + 3], t[i + 4], t[i + 5], t[i + 6]),
        static_cast<int>(t[i + 1]),
        static_cast<int>(t[i + 2]));
      particle->set_generated_mass(t[i + 7]);
      vertices[static_cast<std::size_t>(vertex)]->add_particle_out(particle);
    }

    for (auto const& vtx : vertices) {
      evt.add_vertex(vtx);
    }
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
