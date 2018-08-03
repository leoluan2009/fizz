/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

namespace fizz {
namespace server {

template <CertificateStorage Storage>
constexpr folly::StringPiece TicketCodec<Storage>::Label;

template <CertificateStorage Storage>
Buf TicketCodec<Storage>::encode(ResumptionState resState) {
  Buf selfIdentity = folly::IOBuf::create(0);
  if (resState.serverCert) {
    selfIdentity = folly::IOBuf::copyBuffer(resState.serverCert->getIdentity());
  }

  uint64_t ticketIssueTime = std::chrono::duration_cast<std::chrono::seconds>(
                                 resState.ticketIssueTime.time_since_epoch())
                                 .count();

  auto buf = folly::IOBuf::create(60);
  folly::io::Appender appender(buf.get(), 60);

  fizz::detail::write(resState.version, appender);
  fizz::detail::write(resState.cipher, appender);
  fizz::detail::writeBuf<uint16_t>(resState.resumptionSecret, appender);
  fizz::detail::writeBuf<uint16_t>(selfIdentity, appender);
  appendClientCertificate(Storage, resState.clientCert, appender);
  fizz::detail::write(resState.ticketAgeAdd, appender);
  fizz::detail::write(ticketIssueTime, appender);
  if (resState.alpn) {
    auto alpnBuf = folly::IOBuf::copyBuffer(*resState.alpn);
    fizz::detail::writeBuf<uint8_t>(alpnBuf, appender);
  } else {
    fizz::detail::writeBuf<uint8_t>(nullptr, appender);
  }
  fizz::detail::writeBuf<uint16_t>(resState.appToken, appender);
  return buf;
}

template <CertificateStorage Storage>
ResumptionState TicketCodec<Storage>::decode(
    Buf encoded,
    const FizzServerContext* context) {
  folly::io::Cursor cursor(encoded.get());

  ResumptionState resState;
  fizz::detail::read(resState.version, cursor);
  fizz::detail::read(resState.cipher, cursor);
  fizz::detail::readBuf<uint16_t>(resState.resumptionSecret, cursor);
  Buf selfIdentity;
  fizz::detail::readBuf<uint16_t>(selfIdentity, cursor);

  resState.clientCert = readClientCertificate(cursor);

  fizz::detail::read(resState.ticketAgeAdd, cursor);
  uint64_t seconds;
  fizz::detail::read(seconds, cursor);
  Buf alpnBuf;
  fizz::detail::readBuf<uint8_t>(alpnBuf, cursor);
  if (!alpnBuf->empty()) {
    resState.alpn = alpnBuf->moveToFbString().toStdString();
  }

  resState.ticketIssueTime = std::chrono::time_point<std::chrono::system_clock>(
      std::chrono::seconds(seconds));
  if (context) {
    resState.serverCert =
        context->getCert(selfIdentity->moveToFbString().toStdString());
  }
  if (cursor.isAtEnd()) {
    return resState;
  }
  fizz::detail::readBuf<uint16_t>(resState.appToken, cursor);

  return resState;
}
} // namespace server
} // namespace fizz
