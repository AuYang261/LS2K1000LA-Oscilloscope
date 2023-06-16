mkdir build 
cd build
export PATH=/opt/poky/3.3+snapshot/sysroots/x86_64-pokysdk-linux/usr/bin/loongarch64-poky-linux:$PATH
/opt/poky/3.3+snapshot/sysroots/x86_64-pokysdk-linux/usr/bin/qmake /home/auyang/Loongson/application/GUI/GUI.pro -spec linux-oe-g++ CONFIG+=debug CONFIG+=qml_debug && /usr/bin/make qmake_all
/usr/bin/make -j16
