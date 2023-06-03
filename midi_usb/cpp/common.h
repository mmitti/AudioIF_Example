#pragma once
#define ALIGNMENT_CACHELINE __attribute__((aligned(32)))
#pragma once
#ifndef __HOST_TEST_BUILD__
#include <xil_exception.h>
#endif

template<class F>
inline void execCriticalSection(const F& func){
#ifdef __HOST_TEST_BUILD__
	func();
#else
	volatile u32 mask = mfcpsr();
	if(mask & XIL_EXCEPTION_IRQ){//1だと無効
		func();
	}else{
		Xil_ExceptionDisable();
		func();
		Xil_ExceptionEnable();
	}

#endif
}
