{
  driver: {
    cpp: 'generate_layers',
    layers: {
      event: { parent: 'job', total: 1, starting_number: 0 },
    },
  },
  sources: {
    // Python generator: provides a particle table (creator 'pygen', suffix 'particles')
    pygen: {
      py: 'py_simple_generator_hepmc',
      layer: 'event',
    },
  },
  modules: {
    // Particle table -> HepMC3::GenEvent (creator 'input', suffix 'genevent')
    assemble: {
      cpp: 'genevent_from_table',
      input_layer: 'event',
    },
    convert: {
      cpp: 'convert_hepmcevent_to_data',
      input_layer: 'event',
    },
    // Write data product to output file
    output: {
      cpp: 'form_module',
      products: ['converted'],
    },
  },
}
