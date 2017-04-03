NOSYMT
FORMAT ASCII
TASK FOCAL
INCLUDE /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/crt0.o
;INCLUDE /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/crt0sa.o
;INCLUDE heapstk.o
INCLUDE focal.o
INCLUDE parser.o
FIND /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/libm12.a
FIND /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/libc.a
FIND /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/libsci.a
FIND /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/libhard.a
;FIND /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/libsa.a
END
