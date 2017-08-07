// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// File: decimal.cpp
//

//

#include "common.h"
#include "object.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "decimal.h"
#include "string.h"
#include "decimal_calc.h"

LONG g_OLEAUT32_Loaded = 0;

unsigned int DecDivMod1E9(DECIMAL* value);
void DecMul10(DECIMAL* value);
void DecAddInt32(DECIMAL* value, unsigned int i);

#define COPYDEC(dest, src) {dest = src;}

FCIMPL2_IV(void, COMDecimal::InitSingle, DECIMAL *_this, float value)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    _ASSERTE(_this != NULL);
    HRESULT hr = VarDecFromR4(value, _this);
    if (FAILED(hr))
        FCThrowResVoid(kOverflowException, W("Overflow_Decimal"));
    _this->wReserved = 0;
}
FCIMPLEND

FCIMPL2_IV(void, COMDecimal::InitDouble, DECIMAL *_this, double value)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    _ASSERTE(_this != NULL);
    HRESULT hr = VarDecFromR8(value, _this);
    if (FAILED(hr))
        FCThrowResVoid(kOverflowException, W("Overflow_Decimal"));
    _this->wReserved = 0;
}
FCIMPLEND


#ifdef _MSC_VER
// C4702: unreachable code on IA64 retail
#pragma warning(push)
#pragma warning(disable:4702)
#endif
FCIMPL2(INT32, COMDecimal::DoCompare, DECIMAL * d1, DECIMAL * d2)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    HRESULT hr = VarDecCmp(d1, d2);
    if (FAILED(hr) || (int)hr == VARCMP_NULL) {
        _ASSERTE(!"VarDecCmp failed in Decimal::Compare");
        FCThrowRes(kOverflowException, W("Overflow_Decimal"));
    }
    
    INT32 retVal = ((int)hr) - 1;
    FC_GC_POLL_RET ();
    return retVal;
}
FCIMPLEND
#ifdef _MSC_VER
#pragma warning(pop)
#endif

FCIMPL1(void, COMDecimal::DoFloor, DECIMAL * d)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;
    HRESULT hr;
    hr = VarDecInt(d, &decRes);

    // VarDecInt can't overflow, as of source for OleAut32 build 4265.
    // It only returns NOERROR
    _ASSERTE(hr==NOERROR);

    // copy decRes into d
    COPYDEC(*d, decRes)
    d->wReserved = 0;
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL1(INT32, COMDecimal::GetHashCode, DECIMAL *d)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    _ASSERTE(d != NULL);
    double dbl;
    VarR8FromDec(d, &dbl);
    if (dbl == 0.0) {
        // Ensure 0 and -0 have the same hash code
        return 0;
    }
    // conversion to double is lossy and produces rounding errors so we mask off the lowest 4 bits
    // 
    // For example these two numerically equal decimals with different internal representations produce
    // slightly different results when converted to double:
    //
    // decimal a = new decimal(new int[] { 0x76969696, 0x2fdd49fa, 0x409783ff, 0x00160000 });
    //                     => (decimal)1999021.176470588235294117647000000000 => (double)1999021.176470588
    // decimal b = new decimal(new int[] { 0x3f0f0f0f, 0x1e62edcc, 0x06758d33, 0x00150000 }); 
    //                     => (decimal)1999021.176470588235294117647000000000 => (double)1999021.1764705882
    //
    return ((((int *)&dbl)[0]) & 0xFFFFFFF0) ^ ((int *)&dbl)[1];
}
FCIMPLEND

FCIMPL3(void, COMDecimal::DoMultiply, DECIMAL * d1, DECIMAL * d2, CLR_BOOL * overflowed)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;

    // GC is only triggered for throwing, no need to protect result 
    HRESULT hr = DecimalMul(d1, d2, &decRes);
    if (FAILED(hr)) {
        *overflowed = true;
        FC_GC_POLL();
        return;
    }

    // copy decRes into d1
    COPYDEC(*d1, decRes)
    d1->wReserved = 0;
    *overflowed = false;
    FC_GC_POLL();
} 
FCIMPLEND


FCIMPL2(void, COMDecimal::DoMultiplyThrow, DECIMAL * d1, DECIMAL * d2)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;

    // GC is only triggered for throwing, no need to protect result 
    HRESULT hr = VarDecMul(d1, d2, &decRes);
    if (FAILED(hr)) {
        FCThrowResVoid(kOverflowException, W("Overflow_Decimal"));
    }

    // copy decRes into d1
    COPYDEC(*d1, decRes)
    d1->wReserved = 0;
    FC_GC_POLL();
} 
FCIMPLEND

FCIMPL2(void, COMDecimal::DoRound, DECIMAL * d, INT32 decimals)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;
    
    // GC is only triggered for throwing, no need to protect result 
    if (decimals < 0 || decimals > 28)
        FCThrowArgumentOutOfRangeVoid(W("decimals"), W("ArgumentOutOfRange_DecimalRound"));
    HRESULT hr = VarDecRound(d, decimals, &decRes);
    if (FAILED(hr))
        FCThrowResVoid(kOverflowException, W("Overflow_Decimal"));

    // copy decRes into d
    COPYDEC(*d, decRes)
    d->wReserved = 0;
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL2_IV(void, COMDecimal::DoToCurrency, CY * result, DECIMAL d)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    // GC is only triggered for throwing, no need to protect result
    HRESULT hr = VarCyFromDec(&d, result);
    if (FAILED(hr)) {
        _ASSERTE(hr != E_INVALIDARG);
        FCThrowResVoid(kOverflowException, W("Overflow_Currency"));
    }
}
FCIMPLEND

FCIMPL1(double, COMDecimal::ToDouble, FC_DECIMAL d)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    double result = 0.0;
    // Note: this can fail if the input is an invalid decimal, but for compatibility we should return 0
    VarR8FromDec(&d, &result);
    return result;
}
FCIMPLEND

FCIMPL1(INT32, COMDecimal::ToInt32, FC_DECIMAL d)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL result;
    HRESULT hr = VarDecRound(&d, 0, &result);
    if (FAILED(hr))
        FCThrowRes(kOverflowException, W("Overflow_Decimal"));

    result.wReserved = 0;
    
    if( DECIMAL_SCALE(result) != 0) {
        d = result;
        VarDecFix(&d, &result);
    }

    if (DECIMAL_HI32(result) == 0 && DECIMAL_MID32(result) == 0) {
        INT32 i = DECIMAL_LO32(result);
        if ((INT16)DECIMAL_SIGNSCALE(result) >= 0) {
            if (i >= 0) return i;
        }
        else {
            // Int32.MinValue is represented as sign being negative
            // and Lo32 being 0x80000000 (-ve number). Return that as is without
            // reversing the sign of the number.
            if(i == 0x80000000) return i;
            i = -i;
            if (i <= 0) return i;
        }
    }
    FCThrowRes(kOverflowException, W("Overflow_Int32"));    
}
FCIMPLEND

FCIMPL1(float, COMDecimal::ToSingle, FC_DECIMAL d)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    float result = 0.0f;
    // Note: this can fail if the input is an invalid decimal, but for compatibility we should return 0
    VarR4FromDec(&d, &result);
    return result;
}
FCIMPLEND

FCIMPL1(void, COMDecimal::DoTruncate, DECIMAL * d)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;

    VarDecFix(d, &decRes);

    // copy decRes into d
    COPYDEC(*d, decRes)
    d->wReserved = 0;
    FC_GC_POLL();
}
FCIMPLEND

int COMDecimal::NumberToDecimal(NUMBER* number, DECIMAL* value)
{
    WRAPPER_NO_CONTRACT
    _ASSERTE(number != NULL);
    _ASSERTE(value != NULL);

    DECIMAL d;
    d.wReserved = 0;
    DECIMAL_SIGNSCALE(d) = 0;
    DECIMAL_HI32(d) = 0;
    DECIMAL_LO32(d) = 0;
    DECIMAL_MID32(d) = 0;
    wchar_t* p = number->digits;
    _ASSERT(p != NULL);
    int e = number->scale;
    if (!*p) {
        // To avoid risking an app-compat issue with pre 4.5 (where some app was illegally using Reflection to examine the internal scale bits), we'll only force
        // the scale to 0 if the scale was previously positive
        if (e > 0) {
            e = 0;
        }
    } else {
        if (e > DECIMAL_PRECISION) return 0;
        while ((e > 0 || (*p && e > -28)) &&
                (DECIMAL_HI32(d) < 0x19999999 || (DECIMAL_HI32(d) == 0x19999999 &&
                    (DECIMAL_MID32(d) < 0x99999999 || (DECIMAL_MID32(d) == 0x99999999 &&
                        (DECIMAL_LO32(d) < 0x99999999 || (DECIMAL_LO32(d) == 0x99999999 && *p <= '5'))))))) {
            DecMul10(&d);
            if (*p) DecAddInt32(&d, *p++ - '0');
            e--;
        }
        if (*p++ >= '5') {
            bool round = true;
            if (*(p-1) == '5' && *(p-2) % 2 == 0) { // Check if previous digit is even, only if the when we are unsure whether hows to do Banker's rounding
                                                    // For digits > 5 we will be roundinp up anyway.
                int count = 20; // Look at the next 20 digits to check to round
                while (*p == '0' && count != 0) {
                    p++;
                    count--;
                }
                if (*p == '\0' || count == 0) 
                    round = false;// Do nothing
            }

            if (round) {
                DecAddInt32(&d, 1);
                if ((DECIMAL_HI32(d) | DECIMAL_MID32(d) | DECIMAL_LO32(d)) == 0) {
                    DECIMAL_HI32(d) = 0x19999999;
                    DECIMAL_MID32(d) = 0x99999999;
                    DECIMAL_LO32(d) = 0x9999999A;
                    e++;
                }
            }
        }
    }
    if (e > 0) return 0;
    if (e <= -DECIMAL_PRECISION) 
    {
        // Parsing a large scale zero can give you more precision than fits in the decimal.
        // This should only happen for actual zeros or very small numbers that round to zero.
        DECIMAL_SIGNSCALE(d) = 0;
        DECIMAL_HI32(d) = 0;
        DECIMAL_LO32(d) = 0;
        DECIMAL_MID32(d) = 0;
        DECIMAL_SCALE(d) = (DECIMAL_PRECISION - 1);
    }
    else 
    {
        DECIMAL_SCALE(d) = static_cast<BYTE>(-e);
    }
    DECIMAL_SIGN(d) = number->sign? DECIMAL_NEG: 0;
    *value = d;
    return 1;
}

#if defined(_TARGET_X86_)
        
#pragma warning(disable:4035)

unsigned int DecDivMod1E9(DECIMAL* value)
{
    LIMITED_METHOD_CONTRACT

    _asm {
        mov     ebx,value
        mov     ecx,1000000000
        xor     edx,edx
        mov     eax,[ebx+4]
        div     ecx
        mov     [ebx+4],eax
        mov     eax,[ebx+12]
        div     ecx
        mov     [ebx+12],eax
        mov     eax,[ebx+8]
        div     ecx
        mov     [ebx+8],eax
        mov     eax,edx
    }
}

void DecMul10(DECIMAL* value)
{
    LIMITED_METHOD_CONTRACT

    _asm {
        mov     ebx,value
        mov     eax,[ebx+8]
        mov     edx,[ebx+12]
        mov     ecx,[ebx+4]
        shl     eax,1
        rcl     edx,1
        rcl     ecx,1
        shl     eax,1
        rcl     edx,1
        rcl     ecx,1
        add     eax,[ebx+8]
        adc     edx,[ebx+12]
        adc     ecx,[ebx+4]
        shl     eax,1
        rcl     edx,1
        rcl     ecx,1
        mov     [ebx+8],eax
        mov     [ebx+12],edx
        mov     [ebx+4],ecx
    }
}

void DecAddInt32(DECIMAL* value, unsigned int i)
{
    LIMITED_METHOD_CONTRACT

    _asm {
        mov     edx,value
        mov     eax,i
        add     dword ptr [edx+8],eax
        adc     dword ptr [edx+12],0
        adc     dword ptr [edx+4],0
    }
}

#pragma warning(default:4035)
        
#else // !(defined(_TARGET_X86_)

unsigned int D32DivMod1E9(unsigned int hi32, ULONG* lo32)
{
    LIMITED_METHOD_CONTRACT
    _ASSERTE(lo32 != NULL);

    unsigned __int64 n = (unsigned __int64)hi32 << 32 | *lo32;
    *lo32 = (unsigned int)(n / 1000000000);
    return (unsigned int)(n % 1000000000);
}

unsigned int DecDivMod1E9(DECIMAL* value)
{
    WRAPPER_NO_CONTRACT
    _ASSERTE(value != NULL);

    return D32DivMod1E9(D32DivMod1E9(D32DivMod1E9(0,
        &DECIMAL_HI32(*value)), &DECIMAL_MID32(*value)), &DECIMAL_LO32(*value));
}

void DecShiftLeft(DECIMAL* value)
{
    LIMITED_METHOD_CONTRACT
    _ASSERTE(value != NULL);

    unsigned int c0 = DECIMAL_LO32(*value) & 0x80000000? 1: 0;
    unsigned int c1 = DECIMAL_MID32(*value) & 0x80000000? 1: 0;
    DECIMAL_LO32(*value) <<= 1;
    DECIMAL_MID32(*value) = DECIMAL_MID32(*value) << 1 | c0;
    DECIMAL_HI32(*value) = DECIMAL_HI32(*value) << 1 | c1;
}

int D32AddCarry(ULONG* value, unsigned int i)
{
    LIMITED_METHOD_CONTRACT
    _ASSERTE(value != NULL);

    unsigned int v = *value;
    unsigned int sum = v + i;
    *value = sum;
    return sum < v || sum < i? 1: 0;
}

void DecAdd(DECIMAL* value, DECIMAL* d)
{
    WRAPPER_NO_CONTRACT
    _ASSERTE(value != NULL && d != NULL);

    if (D32AddCarry(&DECIMAL_LO32(*value), DECIMAL_LO32(*d))) {
        if (D32AddCarry(&DECIMAL_MID32(*value), 1)) {
            D32AddCarry(&DECIMAL_HI32(*value), 1);
        }
    }
    if (D32AddCarry(&DECIMAL_MID32(*value), DECIMAL_MID32(*d))) {
        D32AddCarry(&DECIMAL_HI32(*value), 1);
    }
    D32AddCarry(&DECIMAL_HI32(*value), DECIMAL_HI32(*d));
}

void DecMul10(DECIMAL* value)
{
    WRAPPER_NO_CONTRACT
    _ASSERTE(value != NULL);

    DECIMAL d = *value;
    DecShiftLeft(value);
    DecShiftLeft(value);
    DecAdd(value, &d);
    DecShiftLeft(value);
}

void DecAddInt32(DECIMAL* value, unsigned int i)
{
    WRAPPER_NO_CONTRACT
    _ASSERTE(value != NULL);

    if (D32AddCarry(&DECIMAL_LO32(*value), i)) {
        if (D32AddCarry(&DECIMAL_MID32(*value), 1)) {
            D32AddCarry(&DECIMAL_HI32(*value), 1);
        }
    }
}

#endif

#ifdef _PREFAST_
#pragma warning(push)
#pragma warning(disable:21000) // Suppress PREFast warning about overly large function
#endif

FCIMPL2(void, COMDecimal::DoDivideThrow, DECIMAL * pdecL, DECIMAL * pdecR)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;

    // GC is only triggered for throwing, no need to protect result 
    HRESULT hr = VarDecDiv(pdecL, pdecR, &decRes);
    if (FAILED(hr)) {
        if (DISP_E_DIVBYZERO == hr)
        {
            FCThrowVoid(kDivideByZeroException);
        }
        else // hr == DISP_E_OVERFLOW
        {
            FCThrowResVoid(kOverflowException, W("Overflow_Decimal"));
        }
    }

    // copy decRes into d1
    COPYDEC(*pdecL, decRes);
        pdecL->wReserved = 0;
    FC_GC_POLL();
}
FCIMPLEND


FCIMPL3(void, COMDecimal::DoDivide, DECIMAL * pdecL, DECIMAL * pdecR, CLR_BOOL * overflowed)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;

    // GC is only triggered for throwing, no need to protect result 
    HRESULT hr = VarDecDiv(pdecL, pdecR, &decRes);
    if (FAILED(hr)) {
        if (DISP_E_DIVBYZERO == hr)
        {
            FCThrowVoid(kDivideByZeroException);
        }
        else // hr == DISP_E_OVERFLOW
        {
            *overflowed = true;
            FC_GC_POLL();
            return;
        }
    }

    // copy decRes into d1
    COPYDEC(*pdecL, decRes);
    pdecL->wReserved = 0;
    *overflowed = false;
    FC_GC_POLL();
}
FCIMPLEND



#ifdef _PREFAST_
#pragma warning(pop)
#endif
HRESULT DecimalAddSub(__in const DECIMAL * pdecL, __in const DECIMAL * pdecR, __out LPDECIMAL pdecRes, char bSign);

FCIMPL3(void, COMDecimal::DoAddSubThrow, DECIMAL * pdecL, DECIMAL * pdecR, UINT8 bSign)
{
    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;
     
    // GC is only triggered for throwing, no need to protect result 
    HRESULT hr = DecimalAddSub(pdecL, pdecR, &decRes, bSign);
    if (FAILED(hr))
       FCThrowResVoid(kOverflowException, W("Overflow_Decimal"));

    COPYDEC(*pdecL, decRes)
    pdecL->wReserved = 0;
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL4(void, COMDecimal::DoAddSub, DECIMAL * pdecL, DECIMAL * pdecR, UINT8 bSign, CLR_BOOL * overflowed)
{
    FCALL_CONTRACT;

    _ASSERTE(bSign == 0 || bSign == DECIMAL_NEG);

    FCALL_CONTRACT;

    ENSURE_OLEAUT32_LOADED();

    DECIMAL decRes;

    // GC is only triggered for throwing, no need to protect result 
    HRESULT hr = DecimalAddSub(pdecL, pdecR, &decRes, bSign);
    if (FAILED(hr)) {
        *overflowed = true;
        FC_GC_POLL();
        return;
    }

    // copy decRes into d1
    COPYDEC(*pdecL, decRes)
    pdecL->wReserved = 0;
    *overflowed = false;
    FC_GC_POLL();
}
FCIMPLEND
