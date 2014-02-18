mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
mkdir image
make DESTDIR=/home/hotshelf/dev/kssh/build/image install
cd image
tar cvjf kssh-kde4-64bit-$(date +%Y%m%d).tar.bz2 usr

