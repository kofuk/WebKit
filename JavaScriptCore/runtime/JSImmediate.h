/*
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *  Copyright (C) 2006 Alexey Proskuryakov (ap@webkit.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef JSImmediate_h
#define JSImmediate_h

#include <wtf/Assertions.h>
#include <wtf/AlwaysInline.h>
#include <wtf/MathExtras.h>
#include "JSValue.h"
#include <limits>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

namespace JSC {

    class ExecState;
    class JSCell;
    class JSFastMath;
    class JSGlobalData;
    class JSObject;
    class UString;

    JSValuePtr js0();
    JSValuePtr jsNull();
    JSValuePtr jsBoolean(bool b);
    JSValuePtr jsUndefined();
    JSValuePtr jsImpossibleValue();
    JSValuePtr jsNumber(ExecState* exec, double d);
    JSValuePtr jsNumber(ExecState*, char i);
    JSValuePtr jsNumber(ExecState*, unsigned char i);
    JSValuePtr jsNumber(ExecState*, short i);
    JSValuePtr jsNumber(ExecState*, unsigned short i);
    JSValuePtr jsNumber(ExecState* exec, int i);
    JSValuePtr jsNumber(ExecState* exec, unsigned i);
    JSValuePtr jsNumber(ExecState* exec, long i);
    JSValuePtr jsNumber(ExecState* exec, unsigned long i);
    JSValuePtr jsNumber(ExecState* exec, long long i);
    JSValuePtr jsNumber(ExecState* exec, unsigned long long i);
    JSValuePtr jsNumber(JSGlobalData* globalData, double d);
    JSValuePtr jsNumber(JSGlobalData* globalData, short i);
    JSValuePtr jsNumber(JSGlobalData* globalData, unsigned short i);
    JSValuePtr jsNumber(JSGlobalData* globalData, int i);
    JSValuePtr jsNumber(JSGlobalData* globalData, unsigned i);
    JSValuePtr jsNumber(JSGlobalData* globalData, long i);
    JSValuePtr jsNumber(JSGlobalData* globalData, unsigned long i);
    JSValuePtr jsNumber(JSGlobalData* globalData, long long i);
    JSValuePtr jsNumber(JSGlobalData* globalData, unsigned long long i);

    /*
     * A JSValue* is either a pointer to a cell (a heap-allocated object) or an immediate (a type-tagged 
     * value masquerading as a pointer). The low two bits in a JSValue* are available for type tagging
     * because allocator alignment guarantees they will be 00 in cell pointers.
     *
     * For example, on a 32 bit system:
     *
     * JSCell*:             XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                     00
     *                      [ high 30 bits: pointer address ]  [ low 2 bits -- always 0 ]
     * JSImmediate:         XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                     TT
     *                      [ high 30 bits: 'payload' ]             [ low 2 bits -- tag ]
     *
     * Where the bottom two bits are non-zero they either indicate that the immediate is a 31 bit signed
     * integer, or they mark the value as being an immediate of a type other than integer, with a secondary
     * tag used to indicate the exact type.
     *
     * Where the lowest bit is set (TT is equal to 01 or 11) the high 31 bits form a 31 bit signed int value.
     * Where TT is equal to 10 this indicates this is a type of immediate other than an integer, and the next
     * two bits will form an extended tag.
     *
     * 31 bit signed int:   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                     X1
     *                      [ high 30 bits of the value ]      [ high bit part of value ]
     * Other:               YYYYYYYYYYYYYYYYYYYYYYYYYYYY      ZZ               10
     *                      [ extended 'payload' ]  [  extended tag  ]  [  tag 'other'  ]
     *
     * Where the first bit of the extended tag is set this flags the value as being a boolean, and the following
     * bit would flag the value as undefined.  If neither bits are set, the value is null.
     *
     * Other:               YYYYYYYYYYYYYYYYYYYYYYYYYYYY      UB               10
     *                      [ extended 'payload' ]  [ undefined | bool ]  [ tag 'other' ]
     *
     * For boolean value the lowest bit in the payload holds the value of the bool, all remaining bits are zero.
     * For undefined or null immediates the payload is zero.
     *
     * Boolean:             000000000000000000000000000V      01               10
     *                      [ boolean value ]              [ bool ]       [ tag 'other' ]
     * Undefined:           0000000000000000000000000000      10               10
     *                      [ zero ]                    [ undefined ]     [ tag 'other' ]
     * Null:                0000000000000000000000000000      00               10
     *                      [ zero ]                       [ zero ]       [ tag 'other' ]
     */

    /*
     * On 64-bit platforms, we support an alternative encoding form for immediates, if
     * USE(ALTERNATE_JSIMMEDIATE) is defined.
     *
     * The top 16-bits denote the type:
     *
     * Pointer: 0000:PPPP:PPPP:PPPP
     * Integer: FFFF:0000:IIII:IIII
     *
     * 32-bit signed integers are marked with the 16-bit tag '0xFFFF'.  The tag '0x0000'
     * denotes a pointer, or another form of tagged immediate.  Boolean, null and undefined
     * values are encoded in the same manner as the default format.
     */

    class JSImmediate {
    private:
        friend class JIT;
        friend class JSValuePtr;
        friend class JSFastMath;
        friend JSValuePtr js0();
        friend JSValuePtr jsNull();
        friend JSValuePtr jsBoolean(bool b);
        friend JSValuePtr jsUndefined();
        friend JSValuePtr jsImpossibleValue();
        friend JSValuePtr jsNumber(ExecState* exec, double d);
        friend JSValuePtr jsNumber(ExecState*, char i);
        friend JSValuePtr jsNumber(ExecState*, unsigned char i);
        friend JSValuePtr jsNumber(ExecState*, short i);
        friend JSValuePtr jsNumber(ExecState*, unsigned short i);
        friend JSValuePtr jsNumber(ExecState* exec, int i);
        friend JSValuePtr jsNumber(ExecState* exec, unsigned i);
        friend JSValuePtr jsNumber(ExecState* exec, long i);
        friend JSValuePtr jsNumber(ExecState* exec, unsigned long i);
        friend JSValuePtr jsNumber(ExecState* exec, long long i);
        friend JSValuePtr jsNumber(ExecState* exec, unsigned long long i);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, double d);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, short i);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, unsigned short i);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, int i);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, unsigned i);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, long i);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, unsigned long i);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, long long i);
        friend JSValuePtr jsNumber(JSGlobalData* globalData, unsigned long long i);

#if USE(ALTERNATE_JSIMMEDIATE)
        static const intptr_t TagTypeInteger = 0xffff000000000000ll; // bottom bit set indicates integer, this dominates the following bit
#else
        static const intptr_t TagTypeInteger = 0x1; // bottom bit set indicates integer, this dominates the following bit
#endif
        static const intptr_t TagBitTypeOther   = 0x2; // second bit set indicates immediate other than an integer
        static const intptr_t TagMask           = TagTypeInteger | TagBitTypeOther;

        static const intptr_t ExtendedTagMask         = 0xC; // extended tag holds a further two bits
        static const intptr_t ExtendedTagBitBool      = 0x4;
        static const intptr_t ExtendedTagBitUndefined = 0x8;

        static const intptr_t FullTagTypeMask      = TagMask | ExtendedTagMask;
        static const intptr_t FullTagTypeBool      = TagBitTypeOther | ExtendedTagBitBool;
        static const intptr_t FullTagTypeUndefined = TagBitTypeOther | ExtendedTagBitUndefined;
        static const intptr_t FullTagTypeNull      = TagBitTypeOther;

#if USE(ALTERNATE_JSIMMEDIATE)
        static const int32_t IntegerPayloadShift  = 0;
#else
        static const int32_t IntegerPayloadShift  = 1;
#endif
        static const int32_t ExtendedPayloadShift = 4;

        static const intptr_t ExtendedPayloadBitBoolValue = 1 << ExtendedPayloadShift;

        static const int32_t signBit = 0x80000000;
 
        static ALWAYS_INLINE bool isImmediate(JSValuePtr v)
        {
            return rawValue(v) & TagMask;
        }
        
        static ALWAYS_INLINE bool isNumber(JSValuePtr v)
        {
            return rawValue(v) & TagTypeInteger;
        }

        static ALWAYS_INLINE bool isPositiveNumber(JSValuePtr v)
        {
            // A single mask to check for the sign bit and the number tag all at once.
            return (rawValue(v) & (signBit | TagTypeInteger)) == TagTypeInteger;
        }
        
        static ALWAYS_INLINE bool isBoolean(JSValuePtr v)
        {
            return (rawValue(v) & FullTagTypeMask) == FullTagTypeBool;
        }
        
        static ALWAYS_INLINE bool isUndefinedOrNull(JSValuePtr v)
        {
            // Undefined and null share the same value, bar the 'undefined' bit in the extended tag.
            return (rawValue(v) & ~ExtendedTagBitUndefined) == FullTagTypeNull;
        }

        static bool isNegative(JSValuePtr v)
        {
            ASSERT(isNumber(v));
            return rawValue(v) & signBit;
        }

        static JSValuePtr from(char);
        static JSValuePtr from(signed char);
        static JSValuePtr from(unsigned char);
        static JSValuePtr from(short);
        static JSValuePtr from(unsigned short);
        static JSValuePtr from(int);
        static JSValuePtr from(unsigned);
        static JSValuePtr from(long);
        static JSValuePtr from(unsigned long);
        static JSValuePtr from(long long);
        static JSValuePtr from(unsigned long long);
        static JSValuePtr from(double);

        static ALWAYS_INLINE bool isEitherImmediate(JSValuePtr v1, JSValuePtr v2)
        {
            return (rawValue(v1) | rawValue(v2)) & TagMask;
        }

        static ALWAYS_INLINE bool areBothImmediate(JSValuePtr v1, JSValuePtr v2)
        {
            return isImmediate(v1) & isImmediate(v2);
        }

        static ALWAYS_INLINE bool areBothImmediateNumbers(JSValuePtr v1, JSValuePtr v2)
        {
            return rawValue(v1) & rawValue(v2) & TagTypeInteger;
        }

        static double toDouble(JSValuePtr);
        static bool toBoolean(JSValuePtr);
        static JSObject* toObject(JSValuePtr, ExecState*);
        static JSObject* toThisObject(JSValuePtr, ExecState*);
        static UString toString(JSValuePtr);

        static bool getUInt32(JSValuePtr, uint32_t&);
        static bool getTruncatedInt32(JSValuePtr, int32_t&);
        static bool getTruncatedUInt32(JSValuePtr, uint32_t&);

        static int32_t getTruncatedInt32(JSValuePtr);
        static uint32_t getTruncatedUInt32(JSValuePtr);

        static JSValuePtr trueImmediate();
        static JSValuePtr falseImmediate();
        static JSValuePtr undefinedImmediate();
        static JSValuePtr nullImmediate();
        static JSValuePtr zeroImmediate();
        static JSValuePtr oneImmediate();

        static JSValuePtr impossibleValue();
        
        static JSObject* prototype(JSValuePtr, ExecState*);

    private:
#if USE(ALTERNATE_JSIMMEDIATE)
        static const int minImmediateInt = ((-INT_MAX) - 1);
        static const int maxImmediateInt = INT_MAX;
#else
        static const int minImmediateInt = ((-INT_MAX) - 1) >> IntegerPayloadShift;
        static const int maxImmediateInt = INT_MAX >> IntegerPayloadShift;
#endif
        static const unsigned maxImmediateUInt = maxImmediateInt;

        static ALWAYS_INLINE JSValuePtr makeValue(intptr_t integer)
        {
            return JSValuePtr::makeImmediate(integer);
        }

#if USE(ALTERNATE_JSIMMEDIATE)
        static ALWAYS_INLINE JSValuePtr makeInt(uint32_t value)
#else
        static ALWAYS_INLINE JSValuePtr makeInt(int32_t value)
#endif
        {
            return makeValue((static_cast<intptr_t>(value) << IntegerPayloadShift) | TagTypeInteger);
        }
        
        static ALWAYS_INLINE JSValuePtr makeBool(bool b)
        {
            return makeValue((static_cast<intptr_t>(b) << ExtendedPayloadShift) | FullTagTypeBool);
        }
        
        static ALWAYS_INLINE JSValuePtr makeUndefined()
        {
            return makeValue(FullTagTypeUndefined);
        }
        
        static ALWAYS_INLINE JSValuePtr makeNull()
        {
            return makeValue(FullTagTypeNull);
        }
        
        static ALWAYS_INLINE int32_t intValue(JSValuePtr v)
        {
            return static_cast<int32_t>(rawValue(v) >> IntegerPayloadShift);
        }
        
        static ALWAYS_INLINE uint32_t uintValue(JSValuePtr v)
        {
            return static_cast<uint32_t>(rawValue(v) >> IntegerPayloadShift);
        }
        
        static ALWAYS_INLINE bool boolValue(JSValuePtr v)
        {
            return rawValue(v) & ExtendedPayloadBitBoolValue;
        }
        
        static ALWAYS_INLINE intptr_t rawValue(JSValuePtr v)
        {
            return v.immediateValue();
        }

        static double nonInlineNaN();
    };

    ALWAYS_INLINE JSValuePtr JSImmediate::trueImmediate() { return makeBool(true); }
    ALWAYS_INLINE JSValuePtr JSImmediate::falseImmediate() { return makeBool(false); }
    ALWAYS_INLINE JSValuePtr JSImmediate::undefinedImmediate() { return makeUndefined(); }
    ALWAYS_INLINE JSValuePtr JSImmediate::nullImmediate() { return makeNull(); }
    ALWAYS_INLINE JSValuePtr JSImmediate::zeroImmediate() { return makeInt(0); }
    ALWAYS_INLINE JSValuePtr JSImmediate::oneImmediate() { return makeInt(1); }

    // This value is impossible because 0x4 is not a valid pointer but a tag of 0 would indicate non-immediate
    ALWAYS_INLINE JSValuePtr JSImmediate::impossibleValue() { return makeValue(0x4); }

    ALWAYS_INLINE bool JSImmediate::toBoolean(JSValuePtr v)
    {
        ASSERT(isImmediate(v));
        intptr_t bits = rawValue(v);
        return (bits & TagTypeInteger)
            ? bits != TagTypeInteger // !0 ints
            : bits == (FullTagTypeBool | ExtendedPayloadBitBoolValue); // bool true
    }

    ALWAYS_INLINE uint32_t JSImmediate::getTruncatedUInt32(JSValuePtr v)
    {
        ASSERT(isNumber(v));
        return intValue(v);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(char i)
    {
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(signed char i)
    {
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(unsigned char i)
    {
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(short i)
    {
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(unsigned short i)
    {
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(int i)
    {
        if ((i < minImmediateInt) | (i > maxImmediateInt))
            return noValue();
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(unsigned i)
    {
        if (i > maxImmediateUInt)
            return noValue();
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(long i)
    {
        if ((i < minImmediateInt) | (i > maxImmediateInt))
            return noValue();
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(unsigned long i)
    {
        if (i > maxImmediateUInt)
            return noValue();
        return makeInt(i);
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(long long i)
    {
        if ((i < minImmediateInt) | (i > maxImmediateInt))
            return noValue();
        return makeInt(static_cast<intptr_t>(i));
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(unsigned long long i)
    {
        if (i > maxImmediateUInt)
            return noValue();
        return makeInt(static_cast<intptr_t>(i));
    }

    ALWAYS_INLINE JSValuePtr JSImmediate::from(double d)
    {
        const int intVal = static_cast<int>(d);

        if ((intVal < minImmediateInt) | (intVal > maxImmediateInt))
            return noValue();

        // Check for data loss from conversion to int.
        if (intVal != d || (!intVal && signbit(d)))
            return noValue();

        return makeInt(intVal);
    }

    ALWAYS_INLINE int32_t JSImmediate::getTruncatedInt32(JSValuePtr v)
    {
        ASSERT(isNumber(v));
        return intValue(v);
    }

    ALWAYS_INLINE double JSImmediate::toDouble(JSValuePtr v)
    {
        ASSERT(isImmediate(v));
        int i;
        if (isNumber(v))
            i = intValue(v);
        else if (rawValue(v) == FullTagTypeUndefined)
            return nonInlineNaN();
        else
            i = rawValue(v) >> ExtendedPayloadShift;
        return i;
    }

    ALWAYS_INLINE bool JSImmediate::getUInt32(JSValuePtr v, uint32_t& i)
    {
        i = uintValue(v);
        return isPositiveNumber(v);
    }

    ALWAYS_INLINE bool JSImmediate::getTruncatedInt32(JSValuePtr v, int32_t& i)
    {
        i = intValue(v);
        return isNumber(v);
    }

    ALWAYS_INLINE bool JSImmediate::getTruncatedUInt32(JSValuePtr v, uint32_t& i)
    {
        return getUInt32(v, i);
    }

    inline JSValuePtr js0()
    {
        return JSImmediate::zeroImmediate();
    }

    inline JSValuePtr jsNull()
    {
        return JSImmediate::nullImmediate();
    }

    inline JSValuePtr jsBoolean(bool b)
    {
        return b ? JSImmediate::trueImmediate() : JSImmediate::falseImmediate();
    }

    inline JSValuePtr jsUndefined()
    {
        return JSImmediate::undefinedImmediate();
    }

    inline JSValuePtr jsImpossibleValue()
    {
        return JSImmediate::impossibleValue();
    }

    // These are identical logic to the JSValue functions above, and faster than jsNumber(number)->toInt32().
    int32_t toInt32(double);
    uint32_t toUInt32(double);
    int32_t toInt32SlowCase(double, bool& ok);
    uint32_t toUInt32SlowCase(double, bool& ok);

    inline bool JSValuePtr::isUndefined() const
    {
        return asValue() == jsUndefined();
    }

    inline bool JSValuePtr::isNull() const
    {
        return asValue() == jsNull();
    }

    inline bool JSValuePtr::isUndefinedOrNull() const
    {
        return JSImmediate::isUndefinedOrNull(asValue());
    }

    inline bool JSValuePtr::isBoolean() const
    {
        return JSImmediate::isBoolean(asValue());
    }

    inline bool JSValuePtr::getBoolean(bool& v) const
    {
        if (JSImmediate::isBoolean(asValue())) {
            v = JSImmediate::toBoolean(asValue());
            return true;
        }
        
        return false;
    }

    inline bool JSValuePtr::getBoolean() const
    {
        return asValue() == jsBoolean(true);
    }

    ALWAYS_INLINE int32_t JSValuePtr::toInt32(ExecState* exec) const
    {
        int32_t i;
        if (getTruncatedInt32(i))
            return i;
        bool ignored;
        return toInt32SlowCase(toNumber(exec), ignored);
    }

    inline uint32_t JSValuePtr::toUInt32(ExecState* exec) const
    {
        uint32_t i;
        if (getTruncatedUInt32(i))
            return i;
        bool ignored;
        return toUInt32SlowCase(toNumber(exec), ignored);
    }

    inline int32_t toInt32(double val)
    {
        if (!(val >= -2147483648.0 && val < 2147483648.0)) {
            bool ignored;
            return toInt32SlowCase(val, ignored);
        }
        return static_cast<int32_t>(val);
    }

    inline uint32_t toUInt32(double val)
    {
        if (!(val >= 0.0 && val < 4294967296.0)) {
            bool ignored;
            return toUInt32SlowCase(val, ignored);
        }
        return static_cast<uint32_t>(val);
    }

    inline int32_t JSValuePtr::toInt32(ExecState* exec, bool& ok) const
    {
        int32_t i;
        if (getTruncatedInt32(i)) {
            ok = true;
            return i;
        }
        return toInt32SlowCase(toNumber(exec), ok);
    }

    inline uint32_t JSValuePtr::toUInt32(ExecState* exec, bool& ok) const
    {
        uint32_t i;
        if (getTruncatedUInt32(i)) {
            ok = true;
            return i;
        }
        return toUInt32SlowCase(toNumber(exec), ok);
    }

    inline bool JSValuePtr::isCell() const
    {
        return !JSImmediate::isImmediate(asValue());
    }

    inline bool JSValuePtr::isInt32Fast() const
    {
        return JSImmediate::isNumber(asValue());
    }

    inline int32_t JSValuePtr::getInt32Fast() const
    {
        ASSERT(isInt32Fast());
        return JSImmediate::getTruncatedInt32(asValue());
    }

    inline bool JSValuePtr::isUInt32Fast() const
    {
        return JSImmediate::isPositiveNumber(asValue());
    }

    inline uint32_t JSValuePtr::getUInt32Fast() const
    {
        ASSERT(isUInt32Fast());
        return JSImmediate::getTruncatedUInt32(asValue());
    }

    inline JSValuePtr JSValuePtr::makeInt32Fast(int32_t i)
    {
        return JSImmediate::from(i);
    }

    inline bool JSValuePtr::areBothInt32Fast(JSValuePtr v1, JSValuePtr v2)
    {
        return JSImmediate::areBothImmediateNumbers(v1, v2);
    }

    class JSFastMath {
    public:
        static ALWAYS_INLINE bool canDoFastBitwiseOperations(JSValuePtr v1, JSValuePtr v2)
        {
            return JSImmediate::areBothImmediateNumbers(v1, v2);
        }

        static ALWAYS_INLINE JSValuePtr equal(JSValuePtr v1, JSValuePtr v2)
        {
            ASSERT(canDoFastBitwiseOperations(v1, v2));
            return jsBoolean(v1 == v2);
        }

        static ALWAYS_INLINE JSValuePtr notEqual(JSValuePtr v1, JSValuePtr v2)
        {
            ASSERT(canDoFastBitwiseOperations(v1, v2));
            return jsBoolean(v1 != v2);
        }

        static ALWAYS_INLINE JSValuePtr andImmediateNumbers(JSValuePtr v1, JSValuePtr v2)
        {
            ASSERT(canDoFastBitwiseOperations(v1, v2));
            return JSImmediate::makeValue(JSImmediate::rawValue(v1) & JSImmediate::rawValue(v2));
        }

        static ALWAYS_INLINE JSValuePtr xorImmediateNumbers(JSValuePtr v1, JSValuePtr v2)
        {
            ASSERT(canDoFastBitwiseOperations(v1, v2));
            return JSImmediate::makeValue((JSImmediate::rawValue(v1) ^ JSImmediate::rawValue(v2)) | JSImmediate::TagTypeInteger);
        }

        static ALWAYS_INLINE JSValuePtr orImmediateNumbers(JSValuePtr v1, JSValuePtr v2)
        {
            ASSERT(canDoFastBitwiseOperations(v1, v2));
            return JSImmediate::makeValue(JSImmediate::rawValue(v1) | JSImmediate::rawValue(v2));
        }

        static ALWAYS_INLINE bool canDoFastRshift(JSValuePtr v1, JSValuePtr v2)
        {
            return JSImmediate::areBothImmediateNumbers(v1, v2);
        }

        static ALWAYS_INLINE bool canDoFastUrshift(JSValuePtr v1, JSValuePtr v2)
        {
            return JSImmediate::areBothImmediateNumbers(v1, v2) && !JSImmediate::isNegative(v1);
        }

        static ALWAYS_INLINE JSValuePtr rightShiftImmediateNumbers(JSValuePtr val, JSValuePtr shift)
        {
            ASSERT(canDoFastRshift(val, shift) || canDoFastUrshift(val, shift));
#if USE(ALTERNATE_JSIMMEDIATE)
            return JSImmediate::makeValue(static_cast<intptr_t>(static_cast<uint32_t>(static_cast<int32_t>(JSImmediate::rawValue(val)) >> ((JSImmediate::rawValue(shift) >> JSImmediate::IntegerPayloadShift) & 0x1f))) | JSImmediate::TagTypeInteger);
#else
            return JSImmediate::makeValue((JSImmediate::rawValue(val) >> ((JSImmediate::rawValue(shift) >> JSImmediate::IntegerPayloadShift) & 0x1f)) | JSImmediate::TagTypeInteger);
#endif
        }

        static ALWAYS_INLINE bool canDoFastAdditiveOperations(JSValuePtr v)
        {
            // Number is non-negative and an operation involving two of these can't overflow.
            // Checking for allowed negative numbers takes more time than it's worth on SunSpider.
            return (JSImmediate::rawValue(v) & (JSImmediate::TagTypeInteger + (JSImmediate::signBit | (JSImmediate::signBit >> 1)))) == JSImmediate::TagTypeInteger;
        }

        static ALWAYS_INLINE bool canDoFastAdditiveOperations(JSValuePtr v1, JSValuePtr v2)
        {
            // Number is non-negative and an operation involving two of these can't overflow.
            // Checking for allowed negative numbers takes more time than it's worth on SunSpider.
            return ((JSImmediate::rawValue(v1) | JSImmediate::rawValue(v2)) & (JSImmediate::TagTypeInteger + (JSImmediate::signBit | (JSImmediate::signBit >> 1)))) == JSImmediate::TagTypeInteger;
        }

        static ALWAYS_INLINE JSValuePtr addImmediateNumbers(JSValuePtr v1, JSValuePtr v2)
        {
            ASSERT(canDoFastAdditiveOperations(v1, v2));
            return JSImmediate::makeValue(JSImmediate::rawValue(v1) + JSImmediate::rawValue(v2) - JSImmediate::TagTypeInteger);
        }

        static ALWAYS_INLINE JSValuePtr subImmediateNumbers(JSValuePtr v1, JSValuePtr v2)
        {
            ASSERT(canDoFastAdditiveOperations(v1, v2));
            return JSImmediate::makeValue(JSImmediate::rawValue(v1) - JSImmediate::rawValue(v2) + JSImmediate::TagTypeInteger);
        }

        static ALWAYS_INLINE JSValuePtr incImmediateNumber(JSValuePtr v)
        {
            ASSERT(canDoFastAdditiveOperations(v));
            return JSImmediate::makeValue(JSImmediate::rawValue(v) + (1 << JSImmediate::IntegerPayloadShift));
        }

        static ALWAYS_INLINE JSValuePtr decImmediateNumber(JSValuePtr v)
        {
            ASSERT(canDoFastAdditiveOperations(v));
            return JSImmediate::makeValue(JSImmediate::rawValue(v) - (1 << JSImmediate::IntegerPayloadShift));
        }
    };

} // namespace JSC

#endif // JSImmediate_h
