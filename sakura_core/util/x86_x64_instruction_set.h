﻿// InstructionSet.cpp
// Compile by using: cl /EHsc /W4 InstructionSet.cpp
// processor: x86, x64
// Uses the __cpuid intrinsic to get information about
// CPU extended instruction set support.

#include <iostream>
#include <vector>
#include <bitset>
#include <array>
#include <string>
#include <intrin.h>

#include "util/design_template.h"

class InstructionSet final : public TSingleton<InstructionSet>
{
public:
    // getters
    const std::string& Vendor(void) const { return vendor_; }
    const std::string& Brand(void) const { return brand_; }

    bool SSE3(void) const { return f_1_ECX_[0]; }
    bool PCLMULQDQ(void) const { return f_1_ECX_[1]; }
    bool MONITOR(void) const { return f_1_ECX_[3]; }
    bool SSSE3(void) const { return f_1_ECX_[9]; }
    bool FMA(void) const { return f_1_ECX_[12]; }
    bool CMPXCHG16B(void) const { return f_1_ECX_[13]; }
    bool SSE41(void) const { return f_1_ECX_[19]; }
    bool SSE42(void) const { return f_1_ECX_[20]; }
    bool MOVBE(void) const { return f_1_ECX_[22]; }
    bool POPCNT(void) const { return f_1_ECX_[23]; }
    bool AES(void) const { return f_1_ECX_[25]; }
    bool XSAVE(void) const { return f_1_ECX_[26]; }
    bool OSXSAVE(void) const { return f_1_ECX_[27]; }
    bool AVX(void) const { return f_1_ECX_[28]; }
    bool F16C(void) const { return f_1_ECX_[29]; }
    bool RDRAND(void) const { return f_1_ECX_[30]; }

    bool MSR(void) const { return f_1_EDX_[5]; }
    bool CX8(void) const { return f_1_EDX_[8]; }
    bool SEP(void) const { return f_1_EDX_[11]; }
    bool CMOV(void) const { return f_1_EDX_[15]; }
    bool CLFSH(void) const { return f_1_EDX_[19]; }
    bool MMX(void) const { return f_1_EDX_[23]; }
    bool FXSR(void) const { return f_1_EDX_[24]; }
    bool SSE(void) const { return f_1_EDX_[25]; }
    bool SSE2(void) const { return f_1_EDX_[26]; }

    bool FSGSBASE(void) const { return f_7_EBX_[0]; }
    bool BMI1(void) const { return f_7_EBX_[3]; }
    bool HLE(void) const { return isIntel_ && f_7_EBX_[4]; }
    bool AVX2(void) const { return f_7_EBX_[5]; }
    bool BMI2(void) const { return f_7_EBX_[8]; }
    bool ERMS(void) const { return f_7_EBX_[9]; }
    bool INVPCID(void) const { return f_7_EBX_[10]; }
    bool RTM(void) const { return isIntel_ && f_7_EBX_[11]; }
    bool AVX512F(void) const { return f_7_EBX_[16]; }
    bool AVX512DQ(void) const { return f_7_EBX_[17]; }
    bool RDSEED(void) const { return f_7_EBX_[18]; }
    bool ADX(void) const { return f_7_EBX_[19]; }
    bool AVX512IFMA(void) const { return f_7_EBX_[21]; }
    bool AVX512PF(void) const { return f_7_EBX_[26]; }
    bool AVX512ER(void) const { return f_7_EBX_[27]; }
    bool AVX512CD(void) const { return f_7_EBX_[28]; }
    bool SHA(void) const { return f_7_EBX_[29]; }
    bool AVX512BW(void) const { return f_7_EBX_[30]; }
    bool AVX512VL(void) const { return f_7_EBX_[31]; }

    bool PREFETCHWT1(void) const { return f_7_ECX_[0]; }
	bool AVX512VBMI(void) const { return f_7_ECX_[1]; }
	bool AVX512VBMI2(void) const { return f_7_ECX_[6]; }
	bool AVX512VNNI(void) const { return f_7_ECX_[11]; }
	bool AVX512BITALG(void) const { return f_7_ECX_[12]; }
	bool AVX512VPOPCNTDQ(void) const { return f_7_ECX_[14]; }

    bool LAHF(void) const { return f_81_ECX_[0]; }
    bool LZCNT(void) const { return isIntel_ && f_81_ECX_[5]; }
    bool ABM(void) const { return isAMD_ && f_81_ECX_[5]; }
    bool SSE4a(void) const { return isAMD_ && f_81_ECX_[6]; }
    bool XOP(void) const { return isAMD_ && f_81_ECX_[11]; }
    bool TBM(void) const { return isAMD_ && f_81_ECX_[21]; }

    bool SYSCALL(void) const { return isIntel_ && f_81_EDX_[11]; }
    bool MMXEXT(void) const { return isAMD_ && f_81_EDX_[22]; }
    bool RDTSCP(void) const { return isIntel_ && f_81_EDX_[27]; }
    bool _3DNOWEXT(void) const { return isAMD_ && f_81_EDX_[30]; }
    bool _3DNOW(void) const { return isAMD_ && f_81_EDX_[31]; }

	InstructionSet()
        : nIds_{ 0 },
        nExIds_{ 0 },
        isIntel_{ false },
        isAMD_{ false },
        f_1_ECX_{ 0 },
        f_1_EDX_{ 0 },
        f_7_EBX_{ 0 },
        f_7_ECX_{ 0 },
        f_81_ECX_{ 0 },
        f_81_EDX_{ 0 },
        data_{},
        extdata_{}
    {
        //int cpuInfo[4] = {-1};
        std::array<int, 4> cpui;

        // Calling __cpuid with 0x0 as the function_id argument
        // gets the number of the highest valid function ID.
        __cpuid(cpui.data(), 0);
        nIds_ = cpui[0];

        for (int i = 0; i <= nIds_; ++i)
        {
            __cpuidex(cpui.data(), i, 0);
            data_.push_back(cpui);
        }

        // Capture vendor string
        char vendor[0x20];
        memset(vendor, 0, sizeof(vendor));
        *reinterpret_cast<int*>(vendor) = data_[0][1];
        *reinterpret_cast<int*>(vendor + 4) = data_[0][3];
        *reinterpret_cast<int*>(vendor + 8) = data_[0][2];
        vendor_ = vendor;
        if (vendor_ == "GenuineIntel")
        {
            isIntel_ = true;
        }
        else if (vendor_ == "AuthenticAMD")
        {
            isAMD_ = true;
        }

        // load bitset with flags for function 0x00000001
        if (nIds_ >= 1)
        {
            f_1_ECX_ = data_[1][2];
            f_1_EDX_ = data_[1][3];
        }

        // load bitset with flags for function 0x00000007
        if (nIds_ >= 7)
        {
            f_7_EBX_ = data_[7][1];
            f_7_ECX_ = data_[7][2];
        }

        // Calling __cpuid with 0x80000000 as the function_id argument
        // gets the number of the highest valid extended ID.
        __cpuid(cpui.data(), 0x80000000);
        nExIds_ = cpui[0];

        char brand[0x40];
        memset(brand, 0, sizeof(brand));

        for (int i = 0x80000000; i <= nExIds_; ++i)
        {
            __cpuidex(cpui.data(), i, 0);
            extdata_.push_back(cpui);
        }

        // load bitset with flags for function 0x80000001
        if (nExIds_ >= 0x80000001)
        {
            f_81_ECX_ = extdata_[1][2];
            f_81_EDX_ = extdata_[1][3];
        }

        // Interpret CPU brand string if reported
        if (nExIds_ >= 0x80000004)
        {
            memcpy(brand, extdata_[2].data(), sizeof(cpui));
            memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
            memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
            brand_ = brand;
        }
    };

private:
    int nIds_;
    int nExIds_;
    std::string vendor_;
    std::string brand_;
    bool isIntel_;
    bool isAMD_;
    std::bitset<32> f_1_ECX_;
    std::bitset<32> f_1_EDX_;
    std::bitset<32> f_7_EBX_;
    std::bitset<32> f_7_ECX_;
    std::bitset<32> f_81_ECX_;
    std::bitset<32> f_81_EDX_;
    std::vector<std::array<int, 4>> data_;
    std::vector<std::array<int, 4>> extdata_;
};
