#ifndef __BASETHREADDECODERTEST_H__
#define __BASETHREADDECODERTEST_H__

#include "test_stdint.h"
#include <limits.h>
#include <fstream>
#include "codec_api.h"

#include "utils/BufferedData.h"

class BaseThreadDecoderTest {
 public:
  struct Plane {
    const uint8_t* data;
    int width;
    int height;
    int stride;
  };

  struct Frame {
    Plane y;
    Plane u;
    Plane v;
  };

  struct Callback {
    virtual void onDecodeFrame (const Frame& frame) = 0;
  };

  BaseThreadDecoderTest();
  int32_t SetUp();
  void TearDown();
  bool ThreadDecodeFile (const char* fileName, Callback* cbk);

  bool Open (const char* fileName);
  bool DecodeNextFrame (Callback* cbk);
  ISVCDecoder* decoder_;

 private:
  void DecodeFrame (const uint8_t* src, size_t sliceSize, Callback* cbk);
  void FlushFrame (Callback* cbk);

  std::ifstream file_;
  BufferedData buf_;
  BufferedData buf[16];
  uint8_t* pData[3];
  uint8_t* pDst[3];
  int32_t iBufIndex;
  int32_t iThreadCount;
  uint64_t uiTimeStamp;
  enum {\
        OpenFile,
        Decoding,
        EndOfStream,
        End
       } decodeStatus_;
};

#endif //__BASETHREADDECODERTEST_H__
