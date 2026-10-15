#ifndef MCTRUTH_HPP
#define MCTRUTH_HPP

#include "mcparticle.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace examples {
  // Generator-level truth for one event: the generating algorithm and the
  // particles it produced.
  class MCTruth {
  public:
    MCTruth() = default; // required by ROOT I/O
    explicit MCTruth(std::string generator);

    std::string const& Generator() const noexcept;

    void Add(MCParticle particle);
    std::size_t NParticles() const noexcept;
    MCParticle const& GetParticle(std::size_t i) const;
    std::vector<MCParticle> const& Particles() const noexcept;

  private:
    std::string generator_;
    std::vector<MCParticle> particles_;
  };
}

#endif // MCTRUTH_HPP
