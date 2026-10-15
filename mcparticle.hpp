#ifndef MCPARTICLE_HPP
#define MCPARTICLE_HPP

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace examples {
  class MCParticle {
  public:
    MCParticle() = default; // required by ROOT I/O
    MCParticle(int pdgcode, int status, int trackid, int mother,
               std::string process);

    struct MCTrajectory {
      struct MCTrajectoryPt {
        double x, y, z, t, px, py, pz;
      };
      std::vector<MCTrajectoryPt> trajectory_points;
    };
    const int & GetPDG() const;
    const double & Px(size_t i=0) const;
    const double & Py(size_t i=0) const;
    const double & Pz(size_t i=0) const;
    const double & Energy(size_t i=0) const;

    const double & Mass() const;

    void AddTrajectoryPoint(MCTrajectory::MCTrajectoryPt point);
    void SetTrajectory(MCTrajectory trajectory);
    void SetEndProcess(std::string endprocess);

    const int & Status() const;
    const int & Mother() const;
    const int & TrackID() const;

  private:
    int pdgcode_{0},
        status_{0},
        trackid_{-1},
        mother_{-1};
    std::string process_,
        endprocess_{""};
    MCTrajectory trajectory_{};
    std::vector<int> daughters; // track IDs of daughter particles
    double mass_;
    mutable double energy_cache_{0.}; //! backing store for Energy()'s reference; not persisted
  };
}

#endif // MCPARTICLE_HPP
