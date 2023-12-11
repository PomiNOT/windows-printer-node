{
  "targets": [
    {
      "target_name": "printer_module",
      "sources": ["src/utils.cc", "src/printer.cc", "src/job.cc", "src/binding.cc"],
      "include_dirs": ['<!(node -p "require(\'node-addon-api\').include_dir")', "src"],
      "cflags": [ "/EHsc" ],
      "cflags_cc": [ "/EHsc" ],
      "defines": ["NAPI_CPP_EXCEPTIONS"]
    },
    {
        "target_name": "copy_to_root",
        "type": "none",
        "dependencies": ["printer_module"],
        "copies": [
            {
                "files": ["<(PRODUCT_DIR)/printer_module.node"],
                "destination": "."
            }
        ]
    }
  ]
}
