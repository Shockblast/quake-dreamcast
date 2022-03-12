#include <stdio.h>

#define PVR_TXRFMT_VQ_DISABLE		(0 << 30)
#define PVR_TXRFMT_VQ_ENABLE		(1 << 30)
#define PVR_TXRFMT_ARGB1555		(0 << 27)
#define PVR_TXRFMT_RGB565		(1 << 27)
#define PVR_TXRFMT_ARGB4444		(2 << 27)
#define PVR_TXRFMT_YUV422		(3 << 27)
#define PVR_TXRFMT_BUMP			(4 << 27)
#define PVR_TXRFMT_PAL4BPP		(5 << 27)
#define PVR_TXRFMT_PAL8BPP		(6 << 27)
#define PVR_TXRFMT_TWIDDLED		(0 << 26)
#define PVR_TXRFMT_NONTWIDDLED		(1 << 26)
#define PVR_TXRFMT_NOSTRIDE		(0 << 21)
#define PVR_TXRFMT_STRIDE		(1 << 21)


#define	print(name)	printf(#name "=%x\n",name)

int main()
{
	print(PVR_TXRFMT_ARGB1555);
	print(PVR_TXRFMT_RGB565);
	print(PVR_TXRFMT_ARGB4444);
	print(PVR_TXRFMT_PAL8BPP);
	print(PVR_TXRFMT_NONTWIDDLED);
}
