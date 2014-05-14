memleak
=======

#####Simulate a leaky program in a sane, configurable way.

Usage: memleak [-rp] [-bkmg] [-l limit[bkmg]] [-c chunk[bkmg]]
  + -b) Display output in bytes (default).
  + -c) Allocate memory in increments of the specified size. Units are the same as for -l.
  + -g) Display output in gibibytes.
  + -k) Display output in kibibytes.
  + -l) Stop once the specified limit is reached. Units can be B, kB, M, or GB.
  + -m) Display output in mebibytes.
  + -p) Only meaningful with -r. Keep polling malloc once bloating is complete.
  + -r) Keep running and hold onto the address space once bloating is complete.
