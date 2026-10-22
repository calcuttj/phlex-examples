local spd = {
  config: '/home/jacob/phlex-work/sim-sigproc.jsonnet', #'dune/wct/job/sim-sigproc.jsonnet',
  plugins: ['WireCellGen', 'WireCellSigProc', 'WireCellAux', 'WireCellPgraph'],
};

{
  driver: {
    cpp: 'generate_layers',
    layers: {
      event: { parent: 'job', total: 1, starting_number: 0 },
    },
  },
  sources: {

    gun: {
      cpp: 'hmp_gen_event_gun',
      output_layer: 'event',
      pdg: 11,             // e-
      mass: 0.511,         // MeV
      energy: 1000.0,         // MeV kinetic
      direction: [0, 0, 1],
      position: [-1800, 3500, 1300],  // mm, ~center of the drift vol
      number: 1,
      momentum_unit: 'MeV',
      length_unit: 'mm',
    },

    // simple_generate: {
    //   cpp: 'simple_generator_hepmc',
    //   layer: 'event',
    // },
  },
  modules: {

    edep_sim_tracking: {
      cpp: 'esp_tracking',
      input_layer: 'event',
      // gdml: '/home/jacob/Downloads/protodunehd_v8_refactored.gdml',
      gdml: '/home/jacob/phlex-work/dune/xerosere/tests/edep-simphony/geometry/pdhd_like.gdml',
      // gdml: '/home/jacob/phlex-work/dune/xerosere/tests/edep-simphony/geometry/benchmark_small.gdml',
      // edep-sim's preamble leaves optical photons unstacked (killed) on the
      // CPU; turn scintillation stacking on so photons are tracked to the
      // photon-detector shell and land in TG4Event.PhotonDetectors.  (This is
      // the CPU optical path; the GPU plugin path is ddm-p5j.7.)
      macro: '/process/optical/scintillation/setStackPhotons false\n',
    },
    edep_observables: {
      cpp: 'esp_observables',
      input_layer: 'event',
      input_from: 'edep_sim_tracking',  // consumes the TG4Event
    },
    // --- Convert the edep observables' segments to a WCT TrackSegmentSet ---
    segments_to_tracksegmentset: {
      cpp: 'wire_cell_phlex_arrow_edep_segments',
      input_layer: 'event',
    },
    // --- Sample point depos from the segments -----------------------------
    // Ideal lines carry n_electrons directly, so use quanta mode (no recombination).
    sampler: {
      cpp: 'wcph_tracksegmentset_to_deposet',
      inputs: [{ creator: 'segments_to_tracksegmentset', layer: 'event', suffix: 'tracksegmentset' }],
      outputs: [{ suffix: 'deposet' }],
      wct_config: 'tracksegmentset-sampler.jsonnet',
      wct_plugins: ['WireCellGen', 'WireCellPgraph'],
      wct_tla: { ionization: 'recombination' },
    },

    // --- COMMON drift island (deposet -> deposet) -------------------------
    drift: {
      cpp: 'wcph_deposet_filter',
      inputs: [{ creator: 'sampler', layer: 'event', suffix: 'deposet' }],
      outputs: [{ suffix: 'deposet' }],
      wct_config: 'dune/wct/job/drift.jsonnet',
      wct_plugins: ['WireCellGen', 'WireCellAux', 'WireCellPgraph'],
      wct_tla: { detector: 'pdhd', anode_index: '0', input: 'deposet' },
    },

    // // --- CHAIN A: sim + signal processing (pre-drifted) -------------------
    // simsp: {
    //   cpp: 'wcph_deposet_to_frame',
    //   inputs: [{ creator: 'drift', layer: 'event', suffix: 'deposet' }],
    //   outputs: [{ suffix: 'frame' }],
    //   wct_config: spd.config,
    //   wct_plugins: spd.plugins,
    //   wct_tla: { detector: 'pdhd', anode_index: '0', input: 'drifted' },
    // },

    // --- CHAIN A': sim then signal processing, separately (pre-drifted) ---
    sim: {
      cpp: 'wcph_deposet_to_frame',
      inputs: [{ creator: 'drift', layer: 'event', suffix: 'deposet' }],
      outputs: [{ suffix: 'frame' }],
      wct_config: '/home/jacob/phlex-work/sim.jsonnet',
      wct_plugins: ['WireCellGen', 'WireCellSigProc', 'WireCellAux', 'WireCellPgraph'],
      wct_tla: { detector: 'pdhd', anode_index: '0', input: 'drifted' },
    },
    sp: {
      cpp: 'wcph_frame_filter',
      inputs: [{ creator: 'sim', layer: 'event', suffix: 'frame' }],
      outputs: [{ suffix: 'frame' }],
      wct_config: '/home/jacob/phlex-work/sigproc.jsonnet',
      wct_plugins: ['WireCellSigProc', 'WireCellAux', 'WireCellPgraph'],
      wct_tla: { detector: 'pdhd', anode_index: '0' },
    },

    // --- CHAIN B: DepoFluxSplat "true" signal (pre-drifted) ---------------
    splat: {
      cpp: 'wcph_deposet_to_frame',
      inputs: [{ creator: 'drift', layer: 'event', suffix: 'deposet' }],
      outputs: [{ suffix: 'frame' }],
      wct_config: 'dune/wct/job/splat.jsonnet',
      // FieldResponse (used by DepoFluxSplat for origin/drift-speed) is a
      // WireCellSigProc factory, so that plugin is required here too.
      wct_plugins: ['WireCellGen', 'WireCellSigProc', 'WireCellAux', 'WireCellPgraph'],
      wct_tla: { detector: 'pdhd', anode_index: '0', input: 'drifted' },
    },
    a_sampled: {
      cpp: 'wire_cell_phlex_arrow_convert',
      input_layer: 'event',
      input_creator: 'sampler',
      types: ['deposet'],
    },
    a_drifted: {
      cpp: 'wire_cell_phlex_arrow_convert',
      input_layer: 'event',
      input_creator: 'drift',
      types: ['deposet'],
    },
    // a_simsp: {
    //   cpp: 'wire_cell_phlex_arrow_convert',
    //   input_layer: 'event',
    //   input_creator: 'simsp',
    //   types: ['frame'],
    // },
    a_sim: {
      cpp: 'wire_cell_phlex_arrow_convert',
      input_layer: 'event',
      input_creator: 'sim',
      types: ['frame'],
    },
    a_sp: {
      cpp: 'wire_cell_phlex_arrow_convert',
      input_layer: 'event',
      input_creator: 'sp',
      types: ['frame'],
    },
    a_splat: {
      cpp: 'wire_cell_phlex_arrow_convert',
      input_layer: 'event',
      input_creator: 'splat',
      types: ['frame'],
    },
    hdf_out: {
      cpp: 'phlex_arrow_hdf_output',
      output_file: 'obs.h5',  // writes all Arrow-typed products
    },
    // Write "sum" data product to output file
    // output: {
    //   cpp: 'form_module',
    //   products: ['generator_output'],
    // },
  },
}
