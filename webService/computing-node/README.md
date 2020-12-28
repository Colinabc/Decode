# cr-computing-node

#计算模块，该模块目录结构如下：
    --config        //存储初始化文件、异常文件按以及心跳文件
        --cn_init.config
        --cn_start.flag
        --cn_breathe.log
    --include       //项目头文件
    --src           //项目源文件
		

#编译运行(编译或运行前需要软链SDK路径)
    mkdir build
    cd build
    cmake ..
    make -j8
    sudo ./computing


#软链SDK路径
    运行程序前需要将SDK所在绝对路径软链到/usr/local/cr_sdk下，
    软链命令为：ln -s 模型目录绝对路径 /usr/local/cr_sdk