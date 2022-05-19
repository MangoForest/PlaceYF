# PlaceYF
一个辅助装修工具

A tool in ffxiv to help to place furniture.

### 怎么使用
解压缩后，以管理员身份运行 PlaceYF.exe

需要在旋转状态选中家具后使用

### 开发环境
#### 编译器
MSYS2 下的 MinGW-w64
#### 依赖
Nana, gRPC
#### 一些脚本
```shell
$ protoc --proto_path=proto --cpp_out=./proto/ proto/PlaceYF.proto
$ protoc --proto_path=proto --grpc_out=./proto/ --plugin=protoc-gen-grpc=`which grpc_cpp_plugin.exe` proto/PlaceYF.proto

$ ldd libHouseMemory.dll | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" ./
$ ldd PlaceYF.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" ./bin
```


***



[![Sponsor me](https://img.shields.io/badge/Sponsor%20me!-success?logo=wechat&logoColor=white&style=flat-square)](https://pay.mangoforest.xyz/)
