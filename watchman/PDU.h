/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>
#include "watchman/Result.h"
#include "watchman/thirdparty/jansson/jansson.h"

namespace watchman {

class Stream;

enum PduType {
  need_data,
  is_json_compact,
  is_json_pretty,
  is_bser,
  is_bser_v2
};

class PduBuffer {
 public:
  char* buf;
  uint32_t allocd = 0;
  uint32_t rpos = 0;
  uint32_t wpos = 0;
  PduType pdu_type = need_data;
  uint32_t capabilities = 0;

  PduBuffer();
  PduBuffer(const PduBuffer&) = delete;
  PduBuffer(PduBuffer&&) = delete;
  PduBuffer& operator=(const PduBuffer&) = delete;
  PduBuffer& operator=(const PduBuffer&&) = delete;
  ~PduBuffer();

  void clear();
  ResultErrno<folly::Unit>
  jsonEncodeToStream(const json_ref& json, Stream* stm, int flags);
  ResultErrno<folly::Unit> bserEncodeToStream(
      uint32_t bser_version,
      uint32_t bser_capabilities,
      const json_ref& json,
      Stream* stm);

  ResultErrno<folly::Unit> pduEncodeToStream(
      PduType pdu_type,
      uint32_t capabilities,
      const json_ref& json,
      Stream* stm);

  json_ref decodeNext(Stream* stm, json_error_t* jerr);

  /**
   * Read a PDU from `stm`, blocking if necessary, and encode it into
   * stdout through `output_pdu_buf`.
   */
  ResultErrno<folly::Unit> passThru(
      PduType output_pdu,
      uint32_t output_capabilities,
      PduBuffer* output_pdu_buf,
      Stream* stm);

 private:
  bool readAndDetectPdu(Stream* stm, json_error_t* jerr);
  uint32_t shuntDown();
  bool fillBuffer(Stream* stm);
  PduType detectPdu();
  json_ref readJsonPrettyPdu(Stream* stm, json_error_t* jerr);
  json_ref readJsonPdu(Stream* stm, json_error_t* jerr);
  json_ref readBserPdu(Stream* stm, uint32_t bser_version, json_error_t* jerr);
  json_ref decodePdu(Stream* stm, json_error_t* jerr);
  bool decodePduInfo(
      Stream* stm,
      uint32_t bser_version,
      json_int_t* len,
      json_int_t* bser_capabilities,
      json_error_t* jerr);
  bool streamPdu(Stream* stm, json_error_t* jerr);
  bool streamUntilNewLine(Stream* stm);
  bool streamN(Stream* stm, json_int_t len, json_error_t* jerr);
};

} // namespace watchman
