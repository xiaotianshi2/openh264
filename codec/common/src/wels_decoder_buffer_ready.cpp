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

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "crt_util_safe_x.h" // Safe CRT routines like utils for cross platforms

#include "wels_decoder_buffer_ready.h"
#include "utils.h"

WelsDecoderBufferReady::WelsDecoderBufferReady() {
  m_fp = NULL;
  m_pOutputOpt = NULL;
  m_sCtx.pBufferReadyCtx = this;
  m_sCtx.pfCb = StaticDecoderBufferReady;
  m_sCtx.pCodecInstance = NULL;
}

WelsDecoderBufferReady::~WelsDecoderBufferReady() {
  m_fp = NULL;
  m_pOutputOpt = NULL;
}

void WelsDecoderBufferReady::StaticDecoderBufferReady (void* pCtx, void** pDst, void* pDstInfo) {
  WelsDecoderBufferReady* self = (WelsDecoderBufferReady*) pCtx;
  self->DecoderBufferReady (pDst, pDstInfo);
}

void WelsDecoderBufferReady::DecoderBufferReady (void** pDst, void* pDstInfo) {
  if (m_fp != NULL) {
    m_fp (pDst, pDstInfo, m_pOutputOpt);
  }
}

void WelsDecoderBufferReady::SetCodecInstance (void* pCodecInstance) {
  m_sCtx.pCodecInstance = pCodecInstance;
}

void WelsDecoderBufferReady::SetDecoderBufferReadyCallback (WelsDecoderBufferReadyCallback func) {
  m_fp = func;
}

void WelsDecoderBufferReady::SetDecoderBufferReadyOutputOpt (void* pOutputOpt) {
  m_pOutputOpt = pOutputOpt;
}

