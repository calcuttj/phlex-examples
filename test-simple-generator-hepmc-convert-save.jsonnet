{
  driver: {
    cpp: 'generate_layers',
    layers: {
      event: { parent: 'job', total: 1, starting_number: 0 },
    },
  },
  sources: {

    simple_generate: {
      cpp: 'simple_generator_hepmc',
      layer: 'event',
    },
  },
  modules: {

    convert: {
      cpp: 'convert_hepmcevent_to_data',
      input_layer: 'event'
    },
    // Write  data product to output file
    output: {
      cpp: 'form_module',
      products: ['converted'],
    },
  },
}
