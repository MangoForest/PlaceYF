#pragma once

#include <vector>
#include <windows.h>
#include <tlhelp32.h>
#include <grpcpp/grpcpp.h>
#include "PlaceYF.grpc.pb.h"

class PlaceYF{
public:
    explicit PlaceYF(std::string address);
    std::vector<float> getPos();
    void writePos(float x,float y, float z);
    void setPlaceAnywhere(bool f);
    void shutDown();
    static bool Dll_Injection(const wchar_t *dll_name,const char processname[]);
private:
    std::unique_ptr<Position::Stub>stub_;
};


