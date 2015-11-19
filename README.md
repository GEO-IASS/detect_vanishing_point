# detect_vanishing_point
detect vanishing point

先改写main.cpp（没有main函数，只提供几个函数）
编写接口文件detectVanishingP.i （声明main.cpp中的函数）
执行swig -c++ -python detectVanishingP.i
命令会生成detectVanishingP.py 和 detectVanishingP_wrap.cxx
然后把detectVanishingP_wrap.cxx 和 自己的.cpp .h编写Makefile生成_detectVanishingP.so 就可以了
最终依赖的文件是detectVanishingP.py 和 _detectVanishingP.so