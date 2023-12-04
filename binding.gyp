{
  "targets": [
    {
      "target_name": "adder",
      "sources": ["src/binding.cc"],
      "include_dirs": ["<!(node -p 'require(\"node-addon-api\").include_dir')"],
      "cflags": [ "-fexceptions" ],
      "cflags_cc": [ "-fexceptions" ]
    }
  ]
}
