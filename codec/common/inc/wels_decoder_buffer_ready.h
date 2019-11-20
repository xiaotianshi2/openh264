/*!
 * \copy
 *     Copyright (c)  2019, Cisco Systems
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions
 *     are met:
 *
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in
 *          the documentation and/or other materials provided with the
 *          distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *     COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *     BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *     LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *     ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *     POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef WELS_DECODER_BUFFER_READY
#define WELS_DECODER_BUFFER_READY

#include <stdarg.h>
#include "typedefs.h"
#include "utils.h"
#include "codec_app_def.h"
#include "codec_api.h"

class WelsDecoderBufferReady {
 public:
  WelsDecoderBufferReady();
  ~WelsDecoderBufferReady();

  void SetCodecInstance (void* pCodecInstance);
  void SetDecoderBufferReadyCallback (WelsDecoderBufferReadyCallback func);
  void SetDecoderBufferReadyOutputOpt (void* pOutputOpt);

 private:
  static void StaticDecoderBufferReady (void* pCtx, void** pDst, void* pDstInfo);
  void DecoderBufferReady (void** pDst, void* pDstInfo);

  WelsDecoderBufferReadyCallback m_fp;
  void*         m_pOutputOpt;

 public:

  SDecoderBufferReadyCtx m_sCtx;
};

#endif //WELS_DECODER_BUFFER_READY
