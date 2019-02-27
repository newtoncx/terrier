#include "transaction/transaction_thread_context.h"
#include <algorithm>
#include <utility>
#include "transaction/transaction_context.h"

namespace terrier::transaction {
void TransactionThreadContext::AddRunningTxn(timestamp_t start_time) {
  common::SpinLatch::ScopedSpinLatch guard(&curr_running_txns_latch_);
  curr_running_txns_.emplace(start_time);
}

void TransactionThreadContext::RemoveRunningTxn(TransactionContext *const txn) {
  common::SpinLatch::ScopedSpinLatch guard(&curr_running_txns_latch_);
  const size_t ret UNUSED_ATTRIBUTE = curr_running_txns_.erase(txn->StartTime());
  TERRIER_ASSERT(ret == 1, "Transaction did not exist in global transactions table");
  if (gc_enabled_) completed_txns_.push_front(txn);
}

timestamp_t TransactionThreadContext::OldestTransactionStartTime(timestamp_t curr_time) {
  common::SpinLatch::ScopedSpinLatch guard(&curr_running_txns_latch_);
  const auto &oldest_txn = std::min_element(curr_running_txns_.cbegin(), curr_running_txns_.cend());
  const timestamp_t result = (oldest_txn != curr_running_txns_.end()) ? *oldest_txn : curr_time;
  return result;
}

TransactionQueue TransactionThreadContext::CompletedTransactions() {
  common::SpinLatch::ScopedSpinLatch guard(&curr_running_txns_latch_);
  TransactionQueue hand_to_gc;
  while (!completed_txns_.empty()) {
    hand_to_gc.push_front(completed_txns_.front());
    completed_txns_.pop_front();
  }
  TERRIER_ASSERT(completed_txns_.empty(), "TransactionManager's queue should now be empty.");
  return hand_to_gc;
}
}  // namespace terrier::transaction
