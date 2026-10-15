#include "mctruth.hpp"

namespace examples {

  MCTruth::MCTruth(std::string generator) : generator_{std::move(generator)} {}

  std::string const& MCTruth::Generator() const noexcept { return generator_; }

  void MCTruth::Add(MCParticle particle) { particles_.push_back(std::move(particle)); }

  std::size_t MCTruth::NParticles() const noexcept { return particles_.size(); }

  // Throws std::out_of_range if i is not a valid index.
  MCParticle const& MCTruth::GetParticle(std::size_t i) const { return particles_.at(i); }

  std::vector<MCParticle> const& MCTruth::Particles() const noexcept { return particles_; }
}
