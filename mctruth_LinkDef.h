// Selection for the ROOT dictionary that lets FORM write examples::MCTruth.

#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class examples::MCParticle::MCTrajectory::MCTrajectoryPt + ;
#pragma link C++ class std::vector < examples::MCParticle::MCTrajectory::MCTrajectoryPt> + ;
#pragma link C++ class examples::MCParticle::MCTrajectory + ;
#pragma link C++ class examples::MCParticle + ;
#pragma link C++ class std::vector < examples::MCParticle> + ;
#pragma link C++ class examples::MCTruth + ;
#endif
