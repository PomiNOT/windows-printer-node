{
  "targets": [
    {
      "target_name": "printer_module",
      "sources": ["src/utils.cc", "src/binding.cc"],
      "include_dirs": ["node_modules/node-addon-api", "src"],
      "cflags": [ "/EHsc" ],
      "cflags_cc": [ "/EHsc" ]
    }
  ]
}
