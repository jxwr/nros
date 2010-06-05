
all:
	(cd lib; make)
	(cd boot; make)
	tools/mkorzimg b boot.bin nros.img
	(cd setup; make)
	tools/mkorzimg a setup.bin SETUP nros.img
	(cd kernel; make)
	tools/mkorzimg a kernel.bin JXKERNEL nros.img
#test
	(cd app; make)
	tools/mkorzimg a test.bin TEST nros.img
	tools/mkorzimg a idle.bin IDLE nros.img

	tools/mkorzimg a Makefile MAKE nros.img
	tools/mkorzimg a acl.d ACLD nros.img

clean:
	(cd lib; make clean)
	(cd setup; make clean)
	(cd kernel; make clean)
	rm *.bin nros.img
