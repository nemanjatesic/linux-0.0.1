make clean
make
cd apps
./tool_make.sh keyset
./tool_make.sh keyclear
./tool_make.sh encry
./tool_make.sh decry
./tool_make.sh keygen
./tool_make.sh zapocni
cd ..
qemu-system-i386 -hdb hd_oldlinux.img -fda Image -boot a
