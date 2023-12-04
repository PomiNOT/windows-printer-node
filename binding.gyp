{
  "targets": [
    {
      "target_name": "adder",
      "sources": ["src/binding.cc"],
      "include_dirs": ["node_modules/node-addon-api"],
      "cflags": [ "-fexceptions" ],
      "cflags_cc": [ "-fexceptions" ]
    }
  ]
}
