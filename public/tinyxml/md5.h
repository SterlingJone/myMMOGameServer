#pragma once


#include <cstdio>
#include <cstring>

namespace utils {
/*！！！！！！！！！！！！！！！！！！！！！！！！.
| md5 encrypt function
.！！！！！！！！！！！！！！！！！！！！！！！！*/
    void EncryptMD5(unsigned char *output, unsigned char *input, int len);

    void EncryptMD5str(char *output, unsigned char *input, int len);

/*！！！！！！！！！！！！！！！！！！！！！！！！.
| hash encrypt function
.！！！！！！！！！！！！！！！！！！！！！！！！*/
  //  void EncryptHash(char *crypt, char *source);

/*！！！！！！！！！！！！！！！！！！！！！！！！.
| DES encrypt and decrypt function
.！！！！！！！！！！！！！！！！！！！！！！！！*/
   // void EncryptDes(unsigned char *source, unsigned char *encrypt);

   // void DecryptDes(unsigned char *encrypt, unsigned char *source);
}
