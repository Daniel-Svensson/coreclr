// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#pragma once

#ifndef _DECIMAL_CALC_
#define _DECIMAL_CALC_


#if defined(WIN32) && !defined(_TARGET_X64_)
// Use code in oleaut32 for decimal calculations, it is faster

HRESULT DecimalAddSub(__in DECIMAL * pdecL, __in DECIMAL * pdecR, __out LPDECIMAL pdecRes, char bSign)
{
    return (bSign == 0) ? VarDecAdd(pdecL, pdecR, pdecRes) : VarDecSub(pdecL, pdecR, pdecRes);
}

#define DecimalMul VarDecMul
#define DecimalDiv VarDecDiv

#else

#define VarDecAdd DecimalAdd
#define VarDecSub DecimalSub
#define VarDecMul DecimalMul
#define VarDecDiv DecimalDiv

STDAPI DecimalMul(const DECIMAL* l, const DECIMAL *r, DECIMAL * __restrict res);
STDAPI DecimalDiv(const DECIMAL* l, const DECIMAL *r, DECIMAL * __restrict res);
STDAPI DecimalAddSub(_In_ const DECIMAL * pdecL, _In_ const DECIMAL * pdecR, _Out_ DECIMAL * __restrict pdecRes, uint8_t bSign);

inline HRESULT DecimalAdd(const DECIMAL* l, const DECIMAL *r, DECIMAL * __restrict res)
{
    return DecimalAddSub(l, r, res, 0);
}
inline HRESULT DecimalSub(const DECIMAL* l, const DECIMAL *r, DECIMAL * __restrict res)
{
    return DecimalAddSub(l, r, res, DECIMAL_NEG);
}


#endif

#endif // ! _DECIMAL_CALC_
