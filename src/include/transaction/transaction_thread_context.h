#pragma once
#include <unordered_set>
#include <utility>
#include "common/spin_latch.h"
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
   * @param gc_enabled flag to maintain transaction queues for gc
   */
  explicit TransactionThreadContext(worker_id_t worker_id, const bool gc_enabled)
      : worker_id_(worker_id), gc_enabled_(gc_enabled) {}

  /**
   * @return worker id of the thread
   */
  worker_id_t GetWorkerId() const { return worker_id_; }

  /**
   * Adds a transaction to the list of currently running transactions in the thread.
   * @param start_time the timestamp identifying the transaction
   */
  void AddRunningTxn(timestamp_t start_time);

  /**
   * Removes a transaction from the thread context (e.g. due to aboirt or commit).
   * @param txn the context structure for the completed transaction
   */
  void RemoveRunningTxn(TransactionContext *txn);

  /**
   * Returns oldest timestamp of current running transactions in this context.
   * @param curr_time the current logical timestamp at time of function call
   * @return the oldest running transaction timestamp, or current logical time if none
   */
  timestamp_t OldestTransactionStartTime(timestamp_t curr_time);

  /**
   * Produces a queue of completed transactions and removes them from the context.
   * @return queue of committed or aborted transactions
   */
  TransactionQueue CompletedTransactions();

 private:
  // id of the worker thread on which the transaction start and finish.
  worker_id_t worker_id_;

  // Container for running transactions in the thread, stored by start timestamp.
  std::unordered_set<timestamp_t> curr_running_txns_;
  mutable common::SpinLatch curr_running_txns_latch_;

  bool gc_enabled_ = false;
  TransactionQueue completed_txns_;
};
}  // namespace terrier::transaction
