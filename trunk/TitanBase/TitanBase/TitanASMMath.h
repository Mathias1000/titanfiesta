/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

const float pi = 4.0 * atan( 1.0 );
const float half_pi = 0.5 * pi;

float asm_arccos( float r ) {
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	float asm_one = 1.f;
	float asm_half_pi = half_pi;
	__asm {
		fld r
		fld r
		fmul r
		fsubr asm_one
		fsqrt
		fchs
		fdiv
		fld
		fpatan
		fadd asm_half_pi
	}
#else
	return float(acos(r));
#endif
}

float asm_arcsin( float r ) {
	// return arctan( r / sqr( 1.f - r * r ) );
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	const float asm_one = 1.f;
	__asm {
		fld r
		fld r
		fmul r
		fsubr asm_one
		fsqrt
		fdiv
		fld1
		fpatan
	}
#else
	return float(asin(r));
#endif
}

float asm_arctan( float r ) {
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	__asm {
		fld r
		fld1
		fpatan
	}
#else
	return float(atan(r));
#endif
}

float asm_sin( float r ) {
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	__asm {
		fld r
		fsin
	}
#else
	return float(sin(r));
#endif
}

float asm_cos( float r ) {
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	__asm {
		fld r
		fcos
	}
#else
	return float(cos(r));
#endif
}

float asm_tan( float r ) {
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	__asm {
		fld r
		fsin
		fld r
		fcos
		fdiv
	}
#else
	return float(tan(r));
#endif
}

float asm_sqrt( float r ){
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	__asm {
		fld r
		fsqrt
	}
#else
	return float(sqrt(r));
#endif
}

// returns 1 / a for a * a = r
float asm_rsq( float r )
{
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	__asm {
		fld1
		fld r
		fsqrt
		fdiv
	}
#else
	return 1. / sqrt(r);
#endif
}

// returns 1 / a for a * a = r
float apx_rsq( float r ) {
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	const float asm_dot5 = 0.5f;
	const float asm_1dot5 = 1.5f;

	__asm {
		fld r
		fmul asm_dot5
		mov eax, r
		shr eax, 0x1
		neg eax
		add eax, 0x5F400000
		mov r, eax 
		fmul r
		fmul r
		fsubr asm_1dot5
		fmul r
	}
#else
	return 1. / sqrt(r);
#endif
}

#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
__declspec(naked) float __fastcall InvSqrt(float fValue)
{
	__asm {
		mov eax, 0x0be6eb508
		mov dword ptr[esp-12], 0x03fc00000
		sub eax, dword ptr[esp+4]
		sub dword ptr[esp+4], 0x800000
		shr eax, 1
		mov dword ptr[esp-8], eax

		fld dword ptr[esp-8]
		fmul st, st
		fld dword ptr[esp-8]
		fxch st(1)
		fmul dword ptr[esp+4]
		fld dword ptr[esp-12]
		fld st(0)
		fsub st,st(2)

		fld st(1)
		fxch st(1)
		fmul st(3),st
		fmul st(3),st
		fmulp st(4),st
		fsub st,st(2)

		fmul st(2),st
		fmul st(3),st
		fmulp st(2),st
		fxch st(1)
		fsubp st(1),st

		fmulp st(1), st
		ret 4
	}
}
#endif

// returns a random number
FORCEINLINE float asm_rand()
{
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
#	if TITAN_COMP_VER >= 1300
		static unsigned __int64 q = time( NULL );
		_asm {
			movq mm0, q
			pshufw mm1, mm0, 0x1E
			paddd mm0, mm1
			movq q, mm0
			emms
		}
		return float(q);
#	endif
#endif
	return float(rand());
}

// returns the maximum random number
FORCEINLINE float asm_rand_max()
{
	return 9223372036854775807.0f;
}

// returns log2( r ) / log2( e )
float asm_ln( float r ) {    
#if TITAN_COMPILER == TITAN_COMPILER_MSVC && TITAN_ARCH_TYPE == TITAN_ARCHITECTURE_32
	const float asm_e = 2.71828182846f;
	const float asm_1_div_log2_e = .693147180559f;
	const float asm_neg1_div_3 = -.33333333333333333333333333333f;
	const float asm_neg2_div_3 = -.66666666666666666666666666667f;
	const float asm_2 = 2.f;

	int log_2 = 0;

	__asm {
		mov eax, r
		sar eax, 0x17
		and eax, 0xFF
		sub eax, 0x80
		mov log_2, eax
		mov ebx, r
		and ebx, 0x807FFFFF
		add ebx, 0x3F800000
		mov r, ebx
		fld r
		fmul asm_neg1_div_3
		fadd asm_2
		fmul r
		fadd asm_neg2_div_3
		fild log_2
		fadd
		fmul asm_1_div_log2_e
	}
#else
	return log(r);
#endif
}