{
  driver: {
    cpp: 'generate_layers',
    layers: {
      spill: { parent: 'job', total: 10, starting_number: 1 },
    },
  },
  sources: {
    geometry_source: {
      cpp: 'geometry_source',
      geometry_name: 'standard',
    },
    numbers_source: {
      cpp: 'numbers_source',
      layer: 'spill',
    },
  },
  modules: {
    simple_generate: {
      cpp: 'simple_generator',
      layer: 'spill',
    },
    // Write "sum" data product to output file
    output: {
      cpp: 'form_module',
      products: ['generator_output'],
    },
  },
}
