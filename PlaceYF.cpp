#include "PlaceYF.h"

bool PlaceYF::Dll_Injection(const char *dll_name, const char processname[])
{
    char lpdllpath[MAX_PATH];
    char lpdlldir[MAX_PATH];
    GetFullPathName(dll_name, MAX_PATH, lpdllpath, nullptr);
    strcpy(lpdlldir,lpdllpath);
    for(int i= strlen(lpdlldir)-1;i>=0;--i){
        if (lpdlldir[i]!='\\') {
            lpdlldir[i]=0;
        }
        else{
            lpdlldir[i]=0;
            break;
        }
    }
    DWORD processId;
    auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    bool isProcessFound = false;
    bool bRet = Process32First(hSnapshot, &pe);
    HANDLE hVictimProcess = nullptr;
    while (bRet)
    {
        if (strcmp(pe.szExeFile, processname) == 0)
        {
            processId = pe.th32ProcessID;
            hVictimProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, processId);
            if (hVictimProcess != nullptr)
            {
                isProcessFound = true;
                break;
            }
        }
        bRet = Process32Next(hSnapshot, &pe);
    }
    CloseHandle(hSnapshot);
    if (!isProcessFound)
    {
        return false;
    }
    auto size = (strlen(lpdllpath)+1)* sizeof(char );
    auto dirsize=(strlen(lpdlldir)+1)* sizeof(char );
    auto pNameInVictimProcess = VirtualAllocEx(hVictimProcess,nullptr,size,MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    auto pSetDllDir=VirtualAllocEx(hVictimProcess,nullptr,dirsize,MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pNameInVictimProcess == nullptr)
    {
        return false;
    }
    if (pSetDllDir == nullptr)
    {
        return false;
    }
    auto bStatus = WriteProcessMemory(hVictimProcess,pNameInVictimProcess,lpdllpath,size,nullptr);
    if (bStatus == 0)
    {
        return false;
    }
    bStatus = WriteProcessMemory(hVictimProcess,pSetDllDir,lpdlldir,dirsize,nullptr);
    if (bStatus == 0)
    {
        return false;
    }
    auto hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (hKernel32 == nullptr)
    {
        return FALSE;
    }
    auto LoadLibraryAddress = GetProcAddress(hKernel32, "LoadLibraryA");
    auto SetDllDirectoryWAddress = GetProcAddress(hKernel32, "SetDllDirectoryA");
    if (LoadLibraryAddress == nullptr ||SetDllDirectoryWAddress== nullptr)
    {
        return FALSE;
    }
    auto hThreadSet = CreateRemoteThread(hVictimProcess,nullptr,0,(LPTHREAD_START_ROUTINE)SetDllDirectoryWAddress,pSetDllDir,0,nullptr);
    if (hThreadSet == nullptr)
    {
        return false;
    }
    WaitForSingleObject(hThreadSet, INFINITE);

    auto hThreadId = CreateRemoteThread(hVictimProcess,nullptr,0,(LPTHREAD_START_ROUTINE)LoadLibraryAddress,pNameInVictimProcess,0,nullptr);
    if (hThreadId == nullptr)
    {
        return false;
    }
    WaitForSingleObject(hThreadId, INFINITE);
    VirtualFreeEx(hVictimProcess, pNameInVictimProcess, size, MEM_RELEASE);
    VirtualFreeEx(hVictimProcess, pSetDllDir, dirsize, MEM_RELEASE);
    CloseHandle(hVictimProcess);
    return true;
}

void PlaceYF::writePos(float x,float y, float z) {
    grpc::ClientContext ctx;
    Pos pos;
    pos.set_x(x);
    pos.set_y(y);
    pos.set_z(z);
    Empty empty;
    grpc::Status status=stub_->SetPosition(&ctx,pos,&empty);
    if (!status.ok()){
        std::cout << "setPos rpc failed." << std::endl;
    }
}

std::vector<float> PlaceYF::getPos() {
    grpc::ClientContext ctx;
    Empty empty;
    Pos pos;
    grpc::Status status=stub_->GetPosition(&ctx,empty,&pos);
    if (!status.ok()){
//        std::cout << "getPos rpc failed." << std::endl;
        return {0,0,0};
    }
    return {pos.x(),pos.y(),pos.z()};
}

void PlaceYF::setPlaceAnywhere(bool f) {
    grpc::ClientContext ctx;
    SetAny set_any;
    Empty empty;
    set_any.set_set_anywhere(f);
    grpc::Status status=stub_->SetPlaceAnywhere(&ctx,set_any,&empty);
    if (!status.ok()){
        std::cout << "SetPlaceAnywhere rpc failed." << std::endl;
    }
}

PlaceYF::PlaceYF(std::string address) {
    auto ch=grpc::CreateChannel(
            address,
            grpc::InsecureChannelCredentials()
    );
    stub_=Position::NewStub(ch);
}

void PlaceYF::shutDown() {
    grpc::ClientContext ctx;
    Empty req;
    Empty res;
    grpc::Status status=stub_->ShutDown(&ctx,req,&res);
    if (!status.ok()){
        std::cout << "shutdown server failed." << std::endl;
    }
}






