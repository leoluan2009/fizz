/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gmock/gmock.h>

#include <fizz/crypto/KeyDerivation.h>

namespace fizz {

class MockKeyDerivation : public KeyDerivation {
 public:
  MOCK_CONST_METHOD0(hashLength, size_t());
  MOCK_CONST_METHOD0(blankHash, folly::ByteRange());
  MOCK_METHOD4(
      _expandLabel,
      Buf(folly::ByteRange secret,
          folly::StringPiece label,
          Buf& hashValue,
          uint16_t length));
  MOCK_METHOD3(
      _hkdfExpand,
      Buf(folly::ByteRange secret, Buf& info, uint16_t length));
  Buf expandLabel(
      folly::ByteRange secret,
      folly::StringPiece label,
      Buf hashValue,
      uint16_t length) override {
    return _expandLabel(secret, label, hashValue, length);
  }
  Buf hkdfExpand(folly::ByteRange secret, Buf info, uint16_t length) override {
    return _hkdfExpand(secret, info, length);
  }
  MOCK_METHOD3(
      deriveSecret,
      std::vector<uint8_t>(
          folly::ByteRange secret,
          folly::StringPiece label,
          folly::ByteRange messageHash));
  MOCK_METHOD2(
      hkdfExtract,
      std::vector<uint8_t>(folly::ByteRange salt, folly::ByteRange ikm));
  MOCK_METHOD2(hash, void(const folly::IOBuf& in, folly::MutableByteRange out));
  MOCK_METHOD3(
      hmac,
      void(
          folly::ByteRange key,
          const folly::IOBuf& in,
          folly::MutableByteRange out));
};

} // namespace fizz
