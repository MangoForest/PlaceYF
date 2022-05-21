#include "HouseMemory.h"

HouseMemory::HouseMemory() {
    SigScanner sig_scanner=SigScanner();
    layout_ptr = sig_scanner.GetStaticAddressFromSig(
            "48 8B 05 ?? ?? ?? ?? 48 8B 48 20 48 85 C9 74 31 83 B9 ?? ?? ?? ?? ?? 74 28 80 B9 ?? ?? ?? ?? ?? 75 1F 80 B9 ?? ?? ?? ?? ?? 74 03 B0 01 C3",
            2);
    Housingmodule_ptr = sig_scanner.GetStaticAddressFromSig(
            "40 53 48 83 EC 20 33 DB 48 39 1D ?? ?? ?? ?? 75 2C 45 33 C0 33 D2 B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 85 C0 74 11 48 8B C8 E8 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? EB 07",
            0xA);
    housingstruct_ptr = *(intptr_t *) (layout_ptr) + 0x40;
    set_pos_anywhere = sig_scanner.ScanText("C6 87 ?? ?? ?? ?? ?? 4D 8B CC") + 6;
    set_wall_any = sig_scanner.ScanText("C6 87 ?? ?? ?? ?? ?? 80 BF ?? ?? ?? ?? ?? 75 26") + 6;
    set_wallmount_any = sig_scanner.ScanText("74 10 C6 87 ?? ?? ?? ?? ??") + 8;
    showcase_anywhere_rotate = sig_scanner.ScanText("88 87 73 01 00 00 48 8B");
    showcase_anywhere_place = sig_scanner.ScanText("88 87 73 01 00 00 48 83");
}

bool HouseMemory::canEdit() const{
    if(*(intptr_t*)housingstruct_ptr==0)
        return false;
    return *(int32_t*)(*(intptr_t*)housingstruct_ptr) == 2;
}

grpc::Status HouseMemory::GetPosition(grpc::ServerContext *context, const Empty *request, Pos *response) {
    if (*(intptr_t *) housingstruct_ptr != 0) {
        intptr_t item_ptr = *(intptr_t *) (housingstruct_ptr) + 0x18;
        if (*(intptr_t *) item_ptr != 0) {
            auto pos = (float *) (*(intptr_t *) item_ptr + 0x50);
            response->set_x(pos[0]);
            response->set_y(pos[2]);
            response->set_z(pos[1]);
            return grpc::Status::OK;
        }
    }
    return grpc::Status::CANCELLED;
}

grpc::Status HouseMemory::SetPosition(grpc::ServerContext *context, const Pos *request, Empty *response) {
    if(!canEdit())
        return grpc::Status::CANCELLED;
    if(!isIndoor())
        return grpc::Status::CANCELLED;
    try{
        intptr_t item_ptr=*(intptr_t*)(housingstruct_ptr)+0x18;
        if(*(intptr_t*)item_ptr==0)
            return grpc::Status::CANCELLED;
        auto pos = (float*)(*(intptr_t*)item_ptr+0x50);
        pos[0]=request->x();
        pos[1]=request->z();
        pos[2]=request->y();
        return grpc::Status::OK;
    } catch (std::exception &e) {
    }
    return grpc::Status::CANCELLED;
}

grpc::Status HouseMemory::SetPlaceAnywhere(grpc::ServerContext *context, const SetAny *request, Empty *response) {
    if(request->set_anywhere()){
    setPlaceAnywhere(true);
    }else{
        setPlaceAnywhere(false);
    }
    return grpc::Status::OK;
    return grpc::Status::CANCELLED;
}

void HouseMemory::setPlaceAnywhere(bool set_any) const {
    byte state=set_any;
    DWORD oldProtection;
    DWORD _;
    VirtualProtect((LPVOID)set_pos_anywhere, 1, PAGE_EXECUTE_READWRITE, &oldProtection);
    ((byte*)set_pos_anywhere)[0]=state;
    VirtualProtect((LPVOID)set_pos_anywhere,1,oldProtection,&_);

    VirtualProtect((LPVOID)set_wall_any, 1, PAGE_EXECUTE_READWRITE, &oldProtection);
    ((byte*)set_wall_any)[0]=state;
    VirtualProtect((LPVOID)set_wall_any,1,oldProtection,&_);

    VirtualProtect((LPVOID)set_wallmount_any, 1, PAGE_EXECUTE_READWRITE, &oldProtection);
    ((byte*)set_wallmount_any)[0]=state;
    VirtualProtect((LPVOID)set_wallmount_any,1,oldProtection,&_);

    byte *to_write=set_any? new byte[]{0x90, 0x90, 0x90, 0x90, 0x90, 0x90}:new byte[]{0x88, 0x87, 0x73, 0x01, 0x00, 0x00};

    VirtualProtect((LPVOID)showcase_anywhere_rotate, 1, PAGE_EXECUTE_READWRITE,&oldProtection);
    memcpy((byte*)showcase_anywhere_rotate,to_write,6);
    VirtualProtect((LPVOID)showcase_anywhere_rotate, 1, oldProtection, &_);

    VirtualProtect((LPVOID)showcase_anywhere_place, 1, PAGE_EXECUTE_READWRITE,&oldProtection);
    memcpy((byte*)showcase_anywhere_place,to_write,6);
    VirtualProtect((LPVOID)showcase_anywhere_place, 1, oldProtection, &_);
}

grpc::Status HouseMemory::ShutDown(grpc::ServerContext *context, const Empty *request, Empty *response) {
    exit_requested.set_value();
    return grpc::Status::OK;
}

bool HouseMemory::isIndoor() const {
    auto housing_obj_mgr_ptr = *(intptr_t*)Housingmodule_ptr+0x10;
    if(*(intptr_t*)housing_obj_mgr_ptr==0){
        return false;
    }else{
        return true;
    }
}



HMODULE hmodule= nullptr;

void run_server(){
    {
        auto hmemory = HouseMemory();
        std::string server_address("0.0.0.0:8932");
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&hmemory);
        std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
        auto serveFn = [&]() {
            server->Wait();
        };

        std::thread serving_thread(serveFn);

        auto f = hmemory.exit_requested.get_future();
        f.wait();
        server->Shutdown();
        serving_thread.join();
    }
    FreeLibraryAndExitThread(hmodule,0);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            SetEnvironmentVariable("GRPC_VERBOSITY","ERROR");
            SetEnvironmentVariable("GRPC_TRACE","all");
            freopen("_errlog.txt","w",stderr);
            hmodule = hinstDLL;
            auto hthread=CreateThread(nullptr,0,(LPTHREAD_START_ROUTINE)run_server, nullptr,0,nullptr);
            CloseHandle(hthread);
            break;
        }
        case DLL_PROCESS_DETACH:{
            break;
        }
        default:
            break;
    }
    return true;
};
