// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_OPENSSL_UTIL_H_
#define BASE_OPENSSL_UTIL_H_
#pragma once

#include "base/basictypes.h"
#include "base/tracked.h"

namespace base {

// Provides a buffer of at least MIN_SIZE bytes, for use when calling OpenSSL's
// SHA256, HMAC, etc functions, adapting the buffer sizing rules to meet those
// of the our base wrapper APIs.
// This allows the library to write directly to the caller's buffer if it is of
// sufficient size, but if not it will write to temporary |min_sized_buffer_|
// of required size and then its content is automatically copied out on
// destruction, with truncation as appropriate.
template<int MIN_SIZE>
class ScopedOpenSSLSafeSizeBuffer {
 public:
  ScopedOpenSSLSafeSizeBuffer(unsigned char* output, size_t output_len)
      : output_(output),
        output_len_(output_len) {
  }

  ~ScopedOpenSSLSafeSizeBuffer() {
    if (output_len_ < MIN_SIZE) {
      // Copy the temporary buffer out, truncating as needed.
      memcpy(output_, min_sized_buffer_, output_len_);
    }
    // else... any writing already happened directly into |output_|.
  }

  unsigned char* safe_buffer() {
    return output_len_ < MIN_SIZE ? min_sized_buffer_ : output_;
  }

 private:
  // Pointer to the caller's data area and it's associated size, where data
  // written via safe_buffer() will [eventually] end up.
  unsigned char* output_;
  size_t output_len_;

  // Temporary buffer writen into in the case where the caller's
  // buffer is not of sufficient size.
  unsigned char min_sized_buffer_[MIN_SIZE];

  DISALLOW_COPY_AND_ASSIGN(ScopedOpenSSLSafeSizeBuffer);
};

// Drains the OpenSSL ERR_get_error stack. On a debug build the error codes
// are send to VLOG(1), on a release build they are disregarded.
void ClearOpenSSLERRStack();

// Put an instance of this class on the call stack to automatically clear the
// OpenSSL error stack on exit of your function.
class ScopedOpenSSLERRClearer {
 public:
  ScopedOpenSSLERRClearer() {}
  ~ScopedOpenSSLERRClearer() { ClearOpenSSLERRStack(); }

 private:
  DISALLOW_COPY_AND_ASSIGN(ScopedOpenSSLERRClearer);
};

}  // namespace base

#endif  // BASE_OPENSSL_UTIL_H_
