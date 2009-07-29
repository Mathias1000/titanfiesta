#define HEX__(n) 0x##n##LU

/* 8-bit conversion function */
#define B8__(x) ((x&0x0000000FLU)?1:0)      \
               +((x&0x000000F0LU)?2:0)      \
               +((x&0x00000F00LU)?4:0)      \
               +((x&0x0000F000LU)?8:0)      \
               +((x&0x000F0000LU)?16:0)     \
               +((x&0x00F00000LU)?32:0)     \
               +((x&0x0F000000LU)?64:0)     \
               +((x&0xF0000000LU)?128:0)

#define B8(byte1) ((unsigned char)B8__(HEX__(byte1)))
#define B16(byte1,byte2) (((unsigned short)B8(byte1)<<8)+B8(byte2))
#define B32(byte1,byte2,byte3,byte4) (((unsigned long)B8(byte1)<<24) + ((unsigned long)B8(byte2)<<16)  + ((unsigned long)B8(byte3)<<8) + B8(byte4))
#define B64(byte1,byte2,byte3,byte4,byte5,byte6,byte7,byte8) ( ((unsigned long long)B8(byte1)<<56) + ((unsigned long long)B8(byte2)<<48) + ((unsigned long long)B8(byte3)<<40) + ((unsigned long long)B8(byte4)<<32) + ((unsigned long long)B8(byte5)<<24) + ((unsigned long long)B8(byte6)<<16)  + ((unsigned long long)B8(byte7)<<8) + B8(byte8))