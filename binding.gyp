{
  "targets": [
    {
      "target_name": "printer_module",
      "sources": ["src/utils.cc", "src/printer.cc", "src/job.cc", "src/binding.cc"],
      "include_dirs": ["node_modules/node-addon-api", "src"],
      "cflags": [ "/EHsc" ],
      "cflags_cc": [ "/EHsc" ],
      "defines": ["NAPI_CPP_EXCEPTIONS"]
    }
  ]
}
