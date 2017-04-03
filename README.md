Version: 1.0.5
Date: 2017/04/02
Author: David Pitts <http://www.cozx.com/dpitts/>

I. Introduction

FOCAL is an interpretive language that was developed by Digital Equipment 
Corporation (DEC) in the late 1960's for their minicomputers. To learn the 
FOCAL language please refer to the DEC Programming Languages Handbook:

<http://www.bitsavers.org/pdf/dec/pdp8/handbooks/programmingLanguages_May70.pdf>


II. Building FOCAL

To build FOCAL on Linux, z/OS OpenEdition or Solaris:

   `$ make`

To install:

   `$ make install`


To build FOCAL on OpenVMS:

   `$ @make`


III. Running FOCAL

To run FOCAL interactively enter:

```bash
   $ focal
   FOCAL-[VERSION]execution begins
   *
```

FOCAL is now ready for operation.

To run FOCAL with a program:

   `$ focal prog.foc`

This will start FOCAL, read the program, prog.foc, and start it.
