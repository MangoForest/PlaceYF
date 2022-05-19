//
// Created by sensj on 2022/5/6.
//

#ifndef HOOK_UTILS_H
#define HOOK_UTILS_H

#include <string>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>

class SigScanner{
public:
    SigScanner();
    intptr_t GetStaticAddressFromSig(const std::string& str_sig,int offset=0);
    intptr_t ScanText(const std::string& str_sig);
    intptr_t Scan(const std::string& str_sig);
    int IndexOf(std::vector<byte>&need,std::vector<byte>&mask);
private:
    intptr_t base_address_;
    int text_offset_;
    int text_size_;
    int data_offset_;
    int data_size_;
    int rdata_offset_;
    int rdata_size_;
};



#endif //HOOK_UTILS_H
