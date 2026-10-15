#include "mcparticle.hpp"

#include <cmath>
#include <cstdlib>

namespace examples {

  namespace {
    // Rest masses in GeV/c^2 for common particles, keyed by |PDG code|.
    // Unknown codes are treated as massless.
    double mass_from_pdg(int pdgcode)
    {
      switch (std::abs(pdgcode)) {
      case 11:
        return 0.000511; // e
      case 13:
        return 0.105658; // mu
      case 15:
        return 1.77686; // tau
      case 12:
      case 14:
      case 16:
        return 0.; // neutrinos
      case 22:
        return 0.; // photon
      case 111:
        return 0.134977; // pi0
      case 211:
        return 0.139570; // pi+-
      case 130:
        return 0.497611; // K0L
      case 321:
        return 0.493677; // K+-
      case 2112:
        return 0.939565; // neutron
      case 2212:
        return 0.938272; // proton
      case 1000180400:
        return 37.2248; // Ar40
      default:
        return 0.;
      }
    }
  }

  MCParticle::MCParticle(int pdgcode, int status, int trackid, int mother, std::string process) :
    pdgcode_{pdgcode},
    status_{status},
    trackid_{trackid},
    mother_{mother},
    process_{std::move(process)},
    mass_{mass_from_pdg(pdgcode)}
  {
  }

  const int& MCParticle::GetPDG() const { return pdgcode_; }
  const int& MCParticle::Mother() const { return mother_; }
  const int& MCParticle::TrackID() const { return trackid_; }
  const int& MCParticle::Status() const { return status_; }

  // Index into the trajectory; i = 0 is the starting point.
  // Throws std::out_of_range if the point does not exist.
  const double& MCParticle::Px(std::size_t i) const { return trajectory_.trajectory_points.at(i).px; }
  const double& MCParticle::Py(std::size_t i) const { return trajectory_.trajectory_points.at(i).py; }
  const double& MCParticle::Pz(std::size_t i) const { return trajectory_.trajectory_points.at(i).pz; }

  const double& MCParticle::Energy(std::size_t i) const
  {
    auto const& pt = trajectory_.trajectory_points.at(i);
    energy_cache_ = std::sqrt(pt.px * pt.px + pt.py * pt.py + pt.pz * pt.pz + mass_ * mass_);
    return energy_cache_;
  }

  const double& MCParticle::Mass() const { return mass_; }

  void MCParticle::AddTrajectoryPoint(MCTrajectory::MCTrajectoryPt point)
  {
    trajectory_.trajectory_points.push_back(point);
  }

  void MCParticle::SetTrajectory(MCTrajectory trajectory) { trajectory_ = std::move(trajectory); }

  void MCParticle::SetEndProcess(std::string endprocess) { endprocess_ = std::move(endprocess); }
}
