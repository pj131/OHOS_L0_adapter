# OHOS_L0_adapter
一种OpenHarmony L0系统适配方案

目录说明

```shell
.
├── code                                       #源码目录
│   ├── native_system                            #原生系统修改/新增代码
│   │   ├── adapter                                 #适配ohos代码
│   │   ├── cmsis                                   #cmsis接口代码
│   │   ├── demo                                    #ohos示例代码
│   │   ├── ld                                      #链接文件
│   │   ├── mbedtls                                 #mbedtls代码
│   │   └── rtos                                    #freertos代码
│   └── ohos                                     #ohos release3.2修改代码
│       ├── device                                  #device/qemu/L0_xts_demo目录全部代码
│       ├── patchs                                  #ohos修改代码
│       └── vendor                                  #vendor/ohemu/L0_xts_demo目录全部代码
├── images                                       #图片
├── LICENSE                                      #版权声明
├── ohos_l0_adapter.md                           #方案文档
└── README.md
```

