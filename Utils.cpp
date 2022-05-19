//
// Created by sensj on 2022/5/6.
//

#include <stdexcept>
#include "Utils.h"

int SigScanner::IndexOf(std::vector<byte> &need, std::vector<byte> &mask) {
    std::vector<int> bad_shift(256);
    int idx;
    int last = need.size() - 1;
    for (idx = last; idx > 0 && !mask[idx]; --idx);
    int diff = last - idx;
    if (diff == 0)diff = 1;
    for (idx = 0; idx < 256; ++idx)
        bad_shift[idx] = diff;
    for (idx = last - diff; idx < last; ++idx)
        bad_shift[need[idx]] = last - idx;

    int offset = 0;
    last = need.size() - 1;
    int max_offset = text_size_ - need.size();
    byte *buff = (byte *) (text_offset_ + base_address_);
    while (offset <= max_offset) {
        for (int pos = last; need[pos] == (buff + pos)[offset] || mask[pos]; --pos) {
            if (pos == 0)
                return offset;
        }
        offset += bad_shift[(buff + offset)[last]];
    }
    return -1;
}


intptr_t SigScanner::Scan(const std::string &sig) {
    std::string str_sig;
    for(const char&c:sig){
        if(c!=' ')
            str_sig.push_back(c);
    }
    if (str_sig.length() % 2 != 0)
        throw std::invalid_argument("Signature without whitespaces must be divisible by two.");
    std::vector<byte> need(str_sig.length() / 2);
    std::vector<byte> mask(str_sig.length() / 2);
    for (int i = 0; i < need.size(); ++i) {
        std::string s = str_sig.substr(i * 2, 2);
        if (s == "??") {
            need[i] = 0;
            mask[i] = 1;
        } else {
            need[i] = std::stoi(s, nullptr, 16);
            mask[i] = 0;
        }
    }
    int index = IndexOf(need, mask);
    if (index < 0)
        throw std::invalid_argument("Can't find a signature");
    return base_address_ + text_offset_ + index;
}


intptr_t SigScanner::ScanText(const std::string &str_sig) {
    intptr_t scan_ret = Scan(str_sig);
    byte inst_byte = *(byte *) scan_ret;
    if (inst_byte == 0xE8 || inst_byte == 0xE9) {
        int jump_offset = *(int32_t *) (scan_ret + 1);
        scan_ret = scan_ret + jump_offset + 5;
    }
    return scan_ret;
}

std::intptr_t SigScanner::GetStaticAddressFromSig(const std::string &signature, std::int32_t offset)
{
    auto instrAddr = ScanText(signature);
    instrAddr = instrAddr + offset;
    auto bAddr = base_address_;
    std::int64_t num;

    do
    {
        instrAddr = instrAddr + 1;
        num = *(std::int32_t*)instrAddr + static_cast<std::int64_t>(instrAddr) + 4 - bAddr;
    } while (!(num >= data_offset_ && num <= (data_offset_ + data_size_)) && !(num >= rdata_offset_ && num <= (rdata_offset_ + rdata_size_)));

    return instrAddr + *(int32_t *) instrAddr + 4;
}

SigScanner::SigScanner() {
    HANDLE handle = GetCurrentProcess();
    char buffer[1024];
    DWORD buffsize = 1024;
    QueryFullProcessImageNameA(handle, 0, buffer, &buffsize);

    HANDLE tl_snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);
    bool ret = Module32First(tl_snap, &me32);
    bool found = false;
    while (ret) {
        if (strcmp(me32.szModule, "") == 0 || strcmp(me32.szExePath, buffer) == 0) {
            found = true;
            break;
        }
        ret = Module32First(tl_snap, &me32);
    }
    intptr_t text_offset = base_address_+0x3C+0x8;

    base_address_ = (intptr_t) me32.hModule;
    intptr_t new_offset = *(int32_t *) (base_address_ + 0x3C);
    intptr_t header = base_address_ + new_offset;

    intptr_t file_header = header + 4;
    intptr_t num_sections = *(int16_t *) (header + 6);
    intptr_t optional_header = file_header + 20;
    intptr_t section_header = optional_header + 240;
    intptr_t section_cursor = section_header;
    char section_name[8];
    for (int i = 0; i < num_sections; ++i) {
        char*name=(char*)section_cursor;
        if(strcmp(name,".text")==0){
            text_offset_ = *(int32_t *) (section_cursor + 12);
            text_size_ = *(int32_t *) (section_cursor + 8);
        }else if(strcmp(name,".data")==0){
            data_offset_ = *(int32_t *) (section_cursor + 12);
            data_size_ = *(int32_t *) (section_cursor + 8);
        }else if(strcmp(name,".rdata")==0){
            rdata_offset_ = *(int32_t *) (section_cursor + 12);
            rdata_size_ = *(int32_t *) (section_cursor + 8);
        }
        section_cursor += 40;
    }
}

