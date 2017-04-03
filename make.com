$ set verify
$ cc /DEF=(VAXVMS,STRERROR,ANSICRT) focal
$ cc /DEF=(VAXVMS,STRERROR,ANSICRT) parser
$ cc /DEF=(VAXVMS,STRERROR,ANSICRT) screen
$ link focal,parser,screen,runtime.opt/opt
$ purge
$ set noverify
