#pragma once

#include <windows.h>
#include <iostream>
#include <future>
#include <grpcpp/grpcpp.h>
#include "PlaceYF.grpc.pb.h"
#include "Utils.h"

class HouseMemory final:public Position::Service{
public:
    HouseMemory();
    grpc::Status GetPosition(grpc::ServerContext* context, const Empty* request, Pos* response) override;
    grpc::Status SetPosition(grpc::ServerContext* context, const Pos* request, Empty* response) override;
    grpc::Status SetPlaceAnywhere(grpc::ServerContext* context, const SetAny* request, Empty* response) override;
    grpc::Status ShutDown(grpc::ServerContext* context, const Empty* request, Empty* response) override;
    std::promise<void> exit_requested;
private:
    intptr_t layout_ptr;
    intptr_t housingstruct_ptr;
    intptr_t Housingmodule_ptr;
    intptr_t set_pos_anywhere;
    intptr_t set_wall_any;
    intptr_t set_wallmount_any;
    intptr_t showcase_anywhere_rotate;
    intptr_t showcase_anywhere_place;
    bool canEdit() const;
    void setPlaceAnywhere(bool set_any) const;
    bool isIndoor() const;
};


