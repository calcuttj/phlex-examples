#ifndef SIMPLE_GENERATOR_HPP
#define SIMPLE_GENERATOR_HPP

#include "my_geometry.hpp"
#include <HepMC3/GenEvent.h>

namespace examples {
  HepMC3::GenEvent generate(/*geometry const& geom, */int i);
} // namespace examples

#endif // SIMPLE_GENERATOR_HPP
