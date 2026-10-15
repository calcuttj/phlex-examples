Trying to make a simple event generator for phlex example/demo


Need a new source "simple_generator.cpp" that

configuration "test-simple-gen.jsonnet"

generator uses my_geometry. Configuration of the algo depends on the implementation/style of
particle generation: i.e. decays, nu interactions

What about random number generators? Thread safe input like my_geometry?


output of generator:
- could be a "mcparticle" (pdgcode, id, px, py, pz, E)
- or a vector of mcparticles
- or some 'MCTruth' class holding the vector of mcparticles

either way: need an mcparticle class



start with a single generator that transforms a number (not really used) into an mctruth object. 