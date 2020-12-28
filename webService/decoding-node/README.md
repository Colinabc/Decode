#解码模块，该模块目录结构如下：
--config        //存储初始化文件、异常文件按以及心跳文件
----dn_init.config
----dn_start.flag
----dn_breathe.log
----dn_init_local.config
--include       //项目头文件
--src           //项目源文件
#编译运行
mkdir build
cd build
cmake ..
make -j8
sudo ./decoding
