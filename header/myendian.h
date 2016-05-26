#ifndef MYENDIAN_H
#define MYENDIAN_H 

#include <cstring>

using endian = char;
constexpr endian BIGENDIAN=0;
constexpr endian LITTLENDIAN=1;

inline endian systemEndian()
{
    short int number = 0x1;
    char *numPtr = (char*)&number;
    return (numPtr[0] == 1) ? LITTLENDIAN : BIGENDIAN;
}

inline void changeByteEndian(const char* src, char* dst, std::size_t length){
  if (src == nullptr || dst==nullptr) return;
  if (src == dst){
    int i=0; int j=length-1;
    for (; i<j; i++, j--){
       char tmp = dst[i];
       dst[i] = dst[j];
       dst[j] = tmp;
    }
  }else {
    for (int i=0; i< length; i++){
      dst[i]=src[length-1-i];
    }
  }
}

template <endian E>
void toEndian(const char* src, char* dst,  std::size_t  length){
  if (src == nullptr || dst==nullptr) return;
  if (systemEndian() != E){
    return changeByteEndian(src, dst, length);
  }
  if (src == dst) return;
  std::memcpy(dst,src, length);
}

template <class T, endian E>
void toEndianType(const T* src, T* dst){
    if (src == nullptr || dst == nullptr) return;
    const char* src_data = reinterpret_cast<const char*>( src);
    char * dst_data = reinterpret_cast<char*>(dst);
    toEndian<E>(src_data, dst_data, sizeof(T));
}


#endif /* ifndef MYENDIAN */
