#pragma once
#include <unordered_set>
#include <utility>
#include "common/spin_latch.h"
#include "transaction/transaction_context.h"
#include "transaction/transaction_defs.h"

namespace terrier::transaction {
/**
 * A TransactionThreadContext encapsulates information about the thread on which the transaction is started
 * (and presumably will finish). While this is not essential to our concurrency control algorithm, having
 * this information tagged with each transaction helps with various performance optimizations.
 */
class TransactionThreadContext {
 public:
  /**
   * Constructs a new TransactionThreadContext with the given worker_id
   * @param worker_id the worker_id of the thread
   */
  explicit TransactionThreadContext(worker_id_t worker_id, const bool gc_enabled)
      : worker_id_(worker_id), gc_enabled_(gc_enabled) {}

  /**
   * @return worker id of the thread
   */
  worker_id_t GetWorkerId() const { return worker_id_; }

  void AddRunningTxn(timestamp_t start_time);

  void RemoveRunningTxn(TransactionContext *const txn);

  timestamp_t OldestTransactionStartTime();

  TransactionQueue CompletedTransactions();

 private:
  // id of the worker thread on which the transaction start and finish.
  worker_id_t worker_id_;

  // TODO(ncx): new
  std::atomic<timestamp_t> time_{timestamp_t(0)};
  std::unordered_set<timestamp_t> curr_running_txns_;
  mutable common::SpinLatch curr_running_txns_latch_;
  bool gc_enabled_ = false;
  TransactionQueue completed_txns_;
};
}  // namespace terrier::transaction
