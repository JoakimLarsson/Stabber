Stabber - a command line debug format interpreter
===================================================

This is a tool to intermix assembler with source code based on the
STABS debugformat from the AS6809 .LST, .RST and .S files generated
with the -g flag to GCC and -y flag to as6809. Other compilers and
assemblers may also be able to produce STABS, please feel free to 
contribute

The STABS format is used in COFF output files too but in binary form 
I think and not tested. In any case, my 6809 project produces S-records
hence I have no need to add binary debug info support.
 
In general STABS is nowadays replaced by the DWARF debug format and
others but most 8 bit tools are too old to generate DWARF.  

Read more about STABS here: https://docs.freebsd.org/info/stabs/stabs.pdf
 
Compile with:

  gcc -o stabber stabber.c ; 

Use examples:
  
  gcc -S -gstabs helloworld.c
  cat helloworld.s | ./stabber.exe | less

...and in my cross compile case:

  /usr/local/libexec/gcc/m6809-unknown-none/4.3.4/cc1 bouncer1.c -dumpbase bouncer1 -O3 -mint8 -msoft-reg-count=0 -auxbase bouncer1 -o bouncer1.s
  /usr/local/bin/as6809 -l -og -sy bouncer1.s
  cat bouncer1.rst | ./stabber.exe | less	


Have Fun!

/Joakim
