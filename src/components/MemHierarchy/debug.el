(dap-register-debug-template
   "GDB::Run"
   (list :type "gdb"
         :request "launch"
         :name "GDB::Run"
         :args "--interpreter=mi"
         :valuesFormatting "prettyPrinters"
         :target "tb"
         :cwd "/home/sultan/Documents/research/qemu_system_integration/src/components/MemHierarchy"))
