#include <fstream>
#include <gtest/gtest.h>
#include "codec_def.h"
#include "codec_app_def.h"
#include "utils/BufferedData.h"
#include "BaseThreadDecoderTest.h"

static bool ReadFrame (std::ifstream* file, BufferedData* buf) {
  // start code of a frame is {0, 0, 1} or {0, 0, 0, 1}
  char b;

  buf->Clear();
  int32_t sps_count = 0;
  int32_t non_idr_pict_count = 0;
  int32_t idr_pict_count = 0;
  int32_t zeroCount = 0;
  for (;;) {
    file->read (&b, 1);
    if (file->gcount() != 1) { // end of file
      return true;
    }
    if (!buf->PushBack (b)) {
      std::cout << "unable to allocate memory" << std::endl;
      return false;
    }
    if (buf->Length() < 5) {
      continue;
    }
    uint8_t nal_unit_type = 0;
    bool has4ByteStartCode = false;
    bool has3ByteStartCode = false;
    int32_t startcode_len_plus_one = 0;
    if (buf->Length() == 5) {
      if (buf->data()[2] == 1) {
        nal_unit_type = buf->data()[3] & 0x1F;
      } else {
        nal_unit_type = buf->data()[4] & 0x1F;
      }
    } else {
      if (zeroCount < 2) {
        zeroCount = b != 0 ? 0 : zeroCount + 1;
      }
      if (zeroCount == 2) {
        file->read (&b, 1);
        if (file->gcount() != 1) { // end of file
          return true;
        }
        if (!buf->PushBack (b)) {
          std::cout << "unable to allocate memory" << std::endl;
          return false;
        }
        if (b == 1) { //0x000001
          file->read (&b, 1);
          if (file->gcount() != 1) { // end of file
            return true;
          }
          if (!buf->PushBack (b)) {
            std::cout << "unable to allocate memory" << std::endl;
            return false;
          }
          nal_unit_type = b & 0x1F;
          startcode_len_plus_one = 4;
          zeroCount = 0;
        } else if (b == 0) {
          file->read (&b, 1);
          if (file->gcount() != 1) { // end of file
            return true;
          }
          if (!buf->PushBack (b)) {
            std::cout << "unable to allocate memory" << std::endl;
            return false;
          }
          if (b == 1) { //0x00000001
            file->read (&b, 1);
            if (file->gcount() != 1) { // end of file
              return true;
            }
            if (!buf->PushBack (b)) {
              std::cout << "unable to allocate memory" << std::endl;
              return false;
            }
            nal_unit_type = b & 0x1F;
            startcode_len_plus_one = 5;
            zeroCount = 0;
          } else {
            zeroCount = 0;
          }
        } else {
          zeroCount = 0;
        }
      }
    }
    if (nal_unit_type == 1) {
      if (++non_idr_pict_count == 1 && idr_pict_count == 1) {
        file->seekg (-startcode_len_plus_one, file->cur).good();
        buf->SetLength (buf->Length() - startcode_len_plus_one);
        return true;
      }
      if (non_idr_pict_count == 2) {
        file->seekg (-startcode_len_plus_one, file->cur).good();
        buf->SetLength (buf->Length() - startcode_len_plus_one);
        return true;
      }
    } else if (nal_unit_type == 5) {
      if (++idr_pict_count == 1 && non_idr_pict_count == 1) {
        file->seekg (-startcode_len_plus_one, file->cur).good();
        buf->SetLength (buf->Length() - startcode_len_plus_one);
        return true;
      }
      if (idr_pict_count == 2) {
        file->seekg (-startcode_len_plus_one, file->cur).good();
        buf->SetLength (buf->Length() - startcode_len_plus_one);
        return true;
      }
    } else if (nal_unit_type == 7) {
      if ((++sps_count == 1) && (non_idr_pict_count == 1 || idr_pict_count == 1)) {
        file->seekg (-startcode_len_plus_one, file->cur).good();
        buf->SetLength (buf->Length() - startcode_len_plus_one);
        return true;
      }
    }
  }
}

BaseThreadDecoderTest::BaseThreadDecoderTest()
  : decoder_ (NULL), decodeStatus_ (OpenFile), iThreadCount (0), iBufIndex (0), uiTimeStamp (0) {}

int32_t BaseThreadDecoderTest::SetUp() {
  long rv = WelsCreateDecoder (&decoder_);
  EXPECT_EQ (0, rv);
  EXPECT_TRUE (decoder_ != NULL);
  if (decoder_ == NULL) {
    return rv;
  }

  SDecodingParam decParam;
  memset (&decParam, 0, sizeof (SDecodingParam));
  decParam.uiTargetDqLayer = UCHAR_MAX;
  decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
  decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
  int random = rand();
  if (random < 0) random = -random;
  iThreadCount = (2 + random) % 4;
  decoder_->SetOption (DECODER_OPTION_NUM_OF_THREADS, &iThreadCount);

  rv = decoder_->Initialize (&decParam);
  EXPECT_EQ (0, rv);
  return (int32_t)rv;
}

void BaseThreadDecoderTest::TearDown() {
  if (decoder_ != NULL) {
    decoder_->Uninitialize();
    WelsDestroyDecoder (decoder_);
  }
}


void BaseThreadDecoderTest::DecodeFrame (const uint8_t* src, size_t sliceSize, Callback* cbk) {
  SBufferInfo bufInfo;
  memset (pData, 0, sizeof (pData));
  memset (&bufInfo, 0, sizeof (SBufferInfo));
  bufInfo.uiInBsTimeStamp = ++uiTimeStamp;

  DECODING_STATE rv = decoder_->DecodeFrameNoDelay (src, (int) sliceSize, pData, &bufInfo);
  ASSERT_TRUE (rv == dsErrorFree);

  if (bufInfo.iBufferStatus == 1 && cbk != NULL) {
    pDst[0] = pData[0];
    pDst[1] = pData[1];
    pDst[2] = pData[2];
    const Frame frame = {
      {
        // y plane
        pDst[0],
        bufInfo.UsrData.sSystemBuffer.iWidth,
        bufInfo.UsrData.sSystemBuffer.iHeight,
        bufInfo.UsrData.sSystemBuffer.iStride[0]
      },
      {
        // u plane
        pDst[1],
        bufInfo.UsrData.sSystemBuffer.iWidth / 2,
        bufInfo.UsrData.sSystemBuffer.iHeight / 2,
        bufInfo.UsrData.sSystemBuffer.iStride[1]
      },
      {
        // v plane
        pDst[2],
        bufInfo.UsrData.sSystemBuffer.iWidth / 2,
        bufInfo.UsrData.sSystemBuffer.iHeight / 2,
        bufInfo.UsrData.sSystemBuffer.iStride[1]
      },
    };
    //cbk->onDecodeFrame (frame);
  }
}
void BaseThreadDecoderTest::FlushFrame (Callback* cbk) {
  SBufferInfo bufInfo;
  memset (pData, 0, sizeof (pData));
  memset (&bufInfo, 0, sizeof (SBufferInfo));

  DECODING_STATE rv = decoder_->FlushFrame (pData, &bufInfo);
  ASSERT_TRUE (rv == dsErrorFree);

  if (bufInfo.iBufferStatus == 1 && cbk != NULL) {
    pDst[0] = pData[0];
    pDst[1] = pData[1];
    pDst[2] = pData[2];
    const Frame frame = {
      {
        // y plane
        pDst[0],
        bufInfo.UsrData.sSystemBuffer.iWidth,
        bufInfo.UsrData.sSystemBuffer.iHeight,
        bufInfo.UsrData.sSystemBuffer.iStride[0]
      },
      {
        // u plane
        pDst[1],
        bufInfo.UsrData.sSystemBuffer.iWidth / 2,
        bufInfo.UsrData.sSystemBuffer.iHeight / 2,
        bufInfo.UsrData.sSystemBuffer.iStride[1]
      },
      {
        // v plane
        pDst[2],
        bufInfo.UsrData.sSystemBuffer.iWidth / 2,
        bufInfo.UsrData.sSystemBuffer.iHeight / 2,
        bufInfo.UsrData.sSystemBuffer.iStride[1]
      },
    };
    //cbk->onDecodeFrame (frame);
  }
}
bool BaseThreadDecoderTest::ThreadDecodeFile (const char* fileName, Callback* cbk) {
  std::ifstream file (fileName, std::ios::in | std::ios::binary);
  if (!file.is_open())
    return false;

  iBufIndex = 0;
  uiTimeStamp = 0;
  while (true) {
    if (false == ReadFrame (&file, &buf[iBufIndex]))
      return false;
    if (::testing::Test::HasFatalFailure()) {
      return false;
    }
    if (buf[iBufIndex].Length() == 0) {
      break;
    }
    DecodeFrame (buf[iBufIndex].data(), buf[iBufIndex].Length(), cbk);
    if (::testing::Test::HasFatalFailure()) {
      return false;
    }
    if (iThreadCount > 1) {
      if (++iBufIndex >= 16) {
        iBufIndex = 0;
      }
    }
  }

  int32_t iEndOfStreamFlag = 1;
  decoder_->SetOption (DECODER_OPTION_END_OF_STREAM, &iEndOfStreamFlag);

  // Flush out last frames in decoder buffer
  int32_t num_of_frames_in_buffer = 0;
  decoder_->GetOption (DECODER_OPTION_NUM_OF_FRAMES_REMAINING_IN_BUFFER, &num_of_frames_in_buffer);
  for (int32_t i = 0; i < num_of_frames_in_buffer; ++i) {
    FlushFrame (cbk);
  }
  return true;
}

bool BaseThreadDecoderTest::Open (const char* fileName) {
  if (decodeStatus_ == OpenFile) {
    file_.open (fileName, std::ios_base::out | std::ios_base::binary);
    if (file_.is_open()) {
      decodeStatus_ = Decoding;
      return true;
    }
  }
  return false;
}

bool BaseThreadDecoderTest::DecodeNextFrame (Callback* cbk) {
  switch (decodeStatus_) {
  case Decoding:
    if (false == ReadFrame (&file_, &buf_))
      return false;
    if (::testing::Test::HasFatalFailure()) {
      return false;
    }
    if (buf_.Length() == 0) {
      decodeStatus_ = EndOfStream;
      return true;
    }
    DecodeFrame (buf_.data(), buf_.Length(), cbk);
    if (::testing::Test::HasFatalFailure()) {
      return false;
    }
    return true;
  case EndOfStream: {
    int32_t iEndOfStreamFlag = 1;
    decoder_->SetOption (DECODER_OPTION_END_OF_STREAM, &iEndOfStreamFlag);
    DecodeFrame (NULL, 0, cbk);
    decodeStatus_ = End;
    break;
  }
  case OpenFile:
  case End:
    break;
  }
  return false;
}
