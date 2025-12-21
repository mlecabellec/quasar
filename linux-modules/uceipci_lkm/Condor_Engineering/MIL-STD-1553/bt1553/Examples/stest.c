#include <stdio.h>
#include "busapi.h"
#include "apiint.h"

int main(int argc, char** argv)
{
    printf("Size of data types (native):\n");
    printf("  short = %d\n",(unsigned int)sizeof(short));
    printf("  int = %d\n",(unsigned int)sizeof(int));
    printf("  unsigned int = %d\n",(unsigned int)sizeof(unsigned int));
    printf("  long = %d\n",(unsigned int)sizeof(long));
    printf("  unsigned long = %d\n",(unsigned int)sizeof(unsigned long));
    printf("  long long = %d\n",(unsigned int)sizeof(long long));
    printf("  unsigned long long = %d\n",(unsigned int)sizeof(unsigned long long));


    printf("Size of data types (CEI types):\n");
    printf("  CEI_CHAR = %d\n",(unsigned int)sizeof(CEI_CHAR));
    printf("  CEI_UCHAR = %d\n",(unsigned int)sizeof(CEI_UCHAR));
    printf("  CEI_INT16 = %d\n",(unsigned int)sizeof(CEI_INT16));
    printf("  CEI_UINT16 = %d\n",(unsigned int)sizeof(CEI_UINT16));
    printf("  CEI_INT32 = %d\n",(unsigned int)sizeof(CEI_INT32));
    printf("  CEI_UINT32 = %d\n",(unsigned int)sizeof(CEI_UINT32));
    printf("  CEI_INT64 = %d\n",(unsigned int)sizeof(CEI_INT64));
    printf("  CEI_UINT64 = %d\n",(unsigned int)sizeof(CEI_UINT64));
    printf("  CEI_NATIVE_INT = %d\n",(unsigned int)sizeof(CEI_NATIVE_INT));
    printf("  CEI_NATIVE_UINT = %d\n",(unsigned int)sizeof(CEI_NATIVE_UINT));
    printf("  CEI_NATIVE_LONG = %d\n",(unsigned int)sizeof(CEI_NATIVE_LONG));
    printf("  CEI_NATIVE_ULONG = %d\n",(unsigned int)sizeof(CEI_NATIVE_ULONG));
    printf("  CEI_INT = %d\n",(unsigned int)sizeof(CEI_INT));
    printf("  CEI_UINT = %d\n",(unsigned int)sizeof(CEI_UINT));
    printf("  CEI_LONG = %d\n",(unsigned int)sizeof(CEI_LONG));
    printf("  CEI_ULONG = %d\n",(unsigned int)sizeof(CEI_ULONG));


    printf("Size of data types (BusTools/1553 API):\n");
    printf("  BT_INT = %d\n",(unsigned int)sizeof(BT_INT));
    printf("  BT_UINT = %d\n",(unsigned int)sizeof(BT_UINT));
    printf("  BT_U8BIT = %d\n",(unsigned int)sizeof(BT_U8BIT));
    printf("  BT_U16BIT = %d\n",(unsigned int)sizeof(BT_U16BIT));
    printf("  BT_U32BIT = %d\n",(unsigned int)sizeof(BT_U32BIT));
 

    printf("Size of API structures (BusTools/1553 API):\n");
    printf("  BT1553_TIME = %d\n",(unsigned int)sizeof(BT1553_TIME));
    printf("  BC_MESSAGE = %d\n",(unsigned int)sizeof(BC_MESSAGE));
    printf("  BC_CBUF = %d\n",(unsigned int)sizeof(BC_CBUF));
    printf("  BC_DBLOCK = %d\n",(unsigned int)sizeof(BC_DBLOCK));
    printf("  BM_CBUF = %d\n",(unsigned int)sizeof(BM_CBUF));
    printf("  BM_MBUF = %d\n",(unsigned int)sizeof(BM_MBUF));
    printf("  BM_FBUF = %d\n",(unsigned int)sizeof(BM_FBUF));
    printf("  EI_MESSAGE = %d\n",(unsigned int)sizeof(EI_MESSAGE));
    printf("  RT_ABUF_ENTRY = %d\n",(unsigned int)sizeof(RT_ABUF_ENTRY));
    printf("  RT_ABUF = %d\n",(unsigned int)sizeof(RT_ABUF));
    printf("  RT_CBUF = %d\n",(unsigned int)sizeof(RT_CBUF));
    printf("  RT_CBUFBROAD = %d\n",(unsigned int)sizeof(RT_CBUFBROAD));
    printf("  RT_FBUF = %d\n",(unsigned int)sizeof(RT_FBUF));
    printf("  RT_MBUF_HW = %d\n",(unsigned int)sizeof(RT_MBUF_HW));
    printf("  RT_MBUF_API = %d\n",(unsigned int)sizeof(RT_MBUF_API));
    printf("  RT_MBUF = %d\n",(unsigned int)sizeof(RT_MBUF));

    return 0;
}


