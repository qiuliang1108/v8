// Copyright 2020 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_HEAP_CONCURRENT_ALLOCATOR_INL_H_
#define V8_HEAP_CONCURRENT_ALLOCATOR_INL_H_

#include "include/v8-internal.h"
#include "src/common/globals.h"
#include "src/heap/concurrent-allocator.h"
#include "src/heap/heap.h"
#include "src/heap/incremental-marking.h"
#include "src/heap/spaces-inl.h"
#include "src/heap/spaces.h"
#include "src/objects/heap-object.h"

namespace v8 {
namespace internal {

AllocationResult ConcurrentAllocator::Allocate(int object_size,
                                               AllocationAlignment alignment,
                                               AllocationOrigin origin) {
  // TODO(dinfuehr): Add support for allocation observers
  CHECK(FLAG_concurrent_allocation);
  if (object_size > kMaxLabObjectSize) {
    return AllocateOutsideLab(object_size, alignment, origin);
  }

  return AllocateInLab(object_size, alignment, origin);
}

Address ConcurrentAllocator::AllocateOrFail(int object_size,
                                            AllocationAlignment alignment,
                                            AllocationOrigin origin) {
  AllocationResult result = Allocate(object_size, alignment, origin);
  if (!result.IsRetry()) return result.ToObject().address();
  return PerformCollectionAndAllocateAgain(object_size, alignment, origin);
}

AllocationResult ConcurrentAllocator::AllocateInLab(
    int object_size, AllocationAlignment alignment, AllocationOrigin origin) {
  AllocationResult allocation = lab_.AllocateRawAligned(object_size, alignment);
  if (allocation.IsRetry()) {
    return AllocateInLabSlow(object_size, alignment, origin);
  } else {
    return allocation;
  }
}

}  // namespace internal
}  // namespace v8

#endif  // V8_HEAP_CONCURRENT_ALLOCATOR_INL_H_
