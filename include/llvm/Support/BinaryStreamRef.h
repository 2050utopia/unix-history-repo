//===- BinaryStreamRef.h - A copyable reference to a stream -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_BINARYSTREAMREF_H
#define LLVM_SUPPORT_BINARYSTREAMREF_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Support/BinaryStream.h"
#include "llvm/Support/BinaryStreamError.h"
#include "llvm/Support/Error.h"
#include <algorithm>
#include <cstdint>
#include <memory>

namespace llvm {

/// Common stuff for mutable and immutable StreamRefs.
template <class RefType, class StreamType> class BinaryStreamRefBase {
protected:
  BinaryStreamRefBase() = default;
  explicit BinaryStreamRefBase(StreamType &BorrowedImpl)
      : BorrowedImpl(&BorrowedImpl), ViewOffset(0) {
    if (!(BorrowedImpl.getFlags() & BSF_Append))
      Length = BorrowedImpl.getLength();
  }

  BinaryStreamRefBase(std::shared_ptr<StreamType> SharedImpl, uint32_t Offset,
                      Optional<uint32_t> Length)
      : SharedImpl(SharedImpl), BorrowedImpl(SharedImpl.get()),
        ViewOffset(Offset), Length(Length) {}
  BinaryStreamRefBase(StreamType &BorrowedImpl, uint32_t Offset,
                      Optional<uint32_t> Length)
      : BorrowedImpl(&BorrowedImpl), ViewOffset(Offset), Length(Length) {}
  BinaryStreamRefBase(const BinaryStreamRefBase &Other) = default;
  BinaryStreamRefBase &operator=(const BinaryStreamRefBase &Other) = default;

  BinaryStreamRefBase &operator=(BinaryStreamRefBase &&Other) = default;
  BinaryStreamRefBase(BinaryStreamRefBase &&Other) = default;

public:
  llvm::support::endianness getEndian() const {
    return BorrowedImpl->getEndian();
  }

  uint32_t getLength() const {
    if (Length.hasValue())
      return *Length;

    return BorrowedImpl ? (BorrowedImpl->getLength() - ViewOffset) : 0;
  }

  /// Return a new BinaryStreamRef with the first \p N elements removed.  If
  /// this BinaryStreamRef is length-tracking, then the resulting one will be
  /// too.
  RefType drop_front(uint32_t N) const {
    if (!BorrowedImpl)
      return RefType();

    N = std::min(N, getLength());
    RefType Result(static_cast<const RefType &>(*this));
    if (N == 0)
      return Result;

    Result.ViewOffset += N;
    if (Result.Length.hasValue())
      *Result.Length -= N;
    return Result;
  }

  /// Return a new BinaryStreamRef with the last \p N elements removed.  If
  /// this BinaryStreamRef is length-tracking and \p N is greater than 0, then
  /// this BinaryStreamRef will no longer length-track.
  RefType drop_back(uint32_t N) const {
    if (!BorrowedImpl)
      return RefType();

    RefType Result(static_cast<const RefType &>(*this));
    N = std::min(N, getLength());

    if (N == 0)
      return Result;

    // Since we're dropping non-zero bytes from the end, stop length-tracking
    // by setting the length of the resulting StreamRef to an explicit value.
    if (!Result.Length.hasValue())
      Result.Length = getLength();

    *Result.Length -= N;
    return Result;
  }

  /// Return a new BinaryStreamRef with only the first \p N elements remaining.
  RefType keep_front(uint32_t N) const {
    assert(N <= getLength());
    return drop_back(getLength() - N);
  }

  /// Return a new BinaryStreamRef with only the last \p N elements remaining.
  RefType keep_back(uint32_t N) const {
    assert(N <= getLength());
    return drop_front(getLength() - N);
  }

  /// Return a new BinaryStreamRef with the first and last \p N elements
  /// removed.
  RefType drop_symmetric(uint32_t N) const {
    return drop_front(N).drop_back(N);
  }

  /// Return a new BinaryStreamRef with the first \p Offset elements removed,
  /// and retaining exactly \p Len elements.
  RefType slice(uint32_t Offset, uint32_t Len) const {
    return drop_front(Offset).keep_front(Len);
  }

  bool valid() const { return BorrowedImpl != nullptr; }

  bool operator==(const RefType &Other) const {
    if (BorrowedImpl != Other.BorrowedImpl)
      return false;
    if (ViewOffset != Other.ViewOffset)
      return false;
    if (Length != Other.Length)
      return false;
    return true;
  }

protected:
  Error checkOffsetForRead(uint32_t Offset, uint32_t DataSize) const {
    if (Offset > getLength())
      return make_error<BinaryStreamError>(stream_error_code::invalid_offset);
    if (getLength() < DataSize + Offset)
      return make_error<BinaryStreamError>(stream_error_code::stream_too_short);
    return Error::success();
  }

  std::shared_ptr<StreamType> SharedImpl;
  StreamType *BorrowedImpl = nullptr;
  uint32_t ViewOffset = 0;
  Optional<uint32_t> Length;
};

/// BinaryStreamRef is to BinaryStream what ArrayRef is to an Array.  It
/// provides copy-semantics and read only access to a "window" of the underlying
/// BinaryStream. Note that BinaryStreamRef is *not* a BinaryStream.  That is to
/// say, it does not inherit and override the methods of BinaryStream.  In
/// general, you should not pass around pointers or references to BinaryStreams
/// and use inheritance to achieve polymorphism.  Instead, you should pass
/// around BinaryStreamRefs by value and achieve polymorphism that way.
class BinaryStreamRef
    : public BinaryStreamRefBase<BinaryStreamRef, BinaryStream> {
  friend BinaryStreamRefBase<BinaryStreamRef, BinaryStream>;
  friend class WritableBinaryStreamRef;
  BinaryStreamRef(std::shared_ptr<BinaryStream> Impl, uint32_t ViewOffset,
                  Optional<uint32_t> Length)
      : BinaryStreamRefBase(Impl, ViewOffset, Length) {}

public:
  BinaryStreamRef() = default;
  BinaryStreamRef(BinaryStream &Stream);
  BinaryStreamRef(BinaryStream &Stream, uint32_t Offset,
                  Optional<uint32_t> Length);
  explicit BinaryStreamRef(ArrayRef<uint8_t> Data,
                           llvm::support::endianness Endian);
  explicit BinaryStreamRef(StringRef Data, llvm::support::endianness Endian);

  BinaryStreamRef(const BinaryStreamRef &Other) = default;
  BinaryStreamRef &operator=(const BinaryStreamRef &Other) = default;
  BinaryStreamRef(BinaryStreamRef &&Other) = default;
  BinaryStreamRef &operator=(BinaryStreamRef &&Other) = default;

  // Use BinaryStreamRef.slice() instead.
  BinaryStreamRef(BinaryStreamRef &S, uint32_t Offset,
                  uint32_t Length) = delete;

  /// Given an Offset into this StreamRef and a Size, return a reference to a
  /// buffer owned by the stream.
  ///
  /// \returns a success error code if the entire range of data is within the
  /// bounds of this BinaryStreamRef's view and the implementation could read
  /// the data, and an appropriate error code otherwise.
  Error readBytes(uint32_t Offset, uint32_t Size,
                  ArrayRef<uint8_t> &Buffer) const;

  /// Given an Offset into this BinaryStreamRef, return a reference to the
  /// largest buffer the stream could support without necessitating a copy.
  ///
  /// \returns a success error code if implementation could read the data,
  /// and an appropriate error code otherwise.
  Error readLongestContiguousChunk(uint32_t Offset,
                                   ArrayRef<uint8_t> &Buffer) const;
};

struct BinarySubstreamRef {
  uint32_t Offset;            // Offset in the parent stream
  BinaryStreamRef StreamData; // Stream Data

  BinarySubstreamRef slice(uint32_t Off, uint32_t Size) const {
    BinaryStreamRef SubSub = StreamData.slice(Off, Size);
    return {Off + Offset, SubSub};
  }
  BinarySubstreamRef drop_front(uint32_t N) const {
    return slice(N, size() - N);
  }
  BinarySubstreamRef keep_front(uint32_t N) const { return slice(0, N); }

  std::pair<BinarySubstreamRef, BinarySubstreamRef>
  split(uint32_t Offset) const {
    return std::make_pair(keep_front(Offset), drop_front(Offset));
  }

  uint32_t size() const { return StreamData.getLength(); }
  bool empty() const { return size() == 0; }
};

class WritableBinaryStreamRef
    : public BinaryStreamRefBase<WritableBinaryStreamRef,
                                 WritableBinaryStream> {
  friend BinaryStreamRefBase<WritableBinaryStreamRef, WritableBinaryStream>;
  WritableBinaryStreamRef(std::shared_ptr<WritableBinaryStream> Impl,
                          uint32_t ViewOffset, Optional<uint32_t> Length)
      : BinaryStreamRefBase(Impl, ViewOffset, Length) {}

  Error checkOffsetForWrite(uint32_t Offset, uint32_t DataSize) const {
    if (!(BorrowedImpl->getFlags() & BSF_Append))
      return checkOffsetForRead(Offset, DataSize);

    if (Offset > getLength())
      return make_error<BinaryStreamError>(stream_error_code::invalid_offset);
    return Error::success();
  }

public:
  WritableBinaryStreamRef() = default;
  WritableBinaryStreamRef(WritableBinaryStream &Stream);
  WritableBinaryStreamRef(WritableBinaryStream &Stream, uint32_t Offset,
                          Optional<uint32_t> Length);
  explicit WritableBinaryStreamRef(MutableArrayRef<uint8_t> Data,
                                   llvm::support::endianness Endian);
  WritableBinaryStreamRef(const WritableBinaryStreamRef &Other) = default;
  WritableBinaryStreamRef &
  operator=(const WritableBinaryStreamRef &Other) = default;

  WritableBinaryStreamRef(WritableBinaryStreamRef &&Other) = default;
  WritableBinaryStreamRef &operator=(WritableBinaryStreamRef &&Other) = default;

  // Use WritableBinaryStreamRef.slice() instead.
  WritableBinaryStreamRef(WritableBinaryStreamRef &S, uint32_t Offset,
                          uint32_t Length) = delete;

  /// Given an Offset into this WritableBinaryStreamRef and some input data,
  /// writes the data to the underlying stream.
  ///
  /// \returns a success error code if the data could fit within the underlying
  /// stream at the specified location and the implementation could write the
  /// data, and an appropriate error code otherwise.
  Error writeBytes(uint32_t Offset, ArrayRef<uint8_t> Data) const;

  /// Conver this WritableBinaryStreamRef to a read-only BinaryStreamRef.
  operator BinaryStreamRef() const;

  /// For buffered streams, commits changes to the backing store.
  Error commit();
};

} // end namespace llvm

#endif // LLVM_SUPPORT_BINARYSTREAMREF_H
