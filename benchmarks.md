## Baseline (Nov 27)
- Input: gen.exe seed 1, 1,000,000 lines (820,416 orders, rest cancels)
- Measures: reading + parsing + matching (end to end)
- Median of 5 runs: 1.391 s → ~590,000 orders/sec
- Range: 1.124 – 1.674 s

## Load and matching timed separately (Nov 27)
- Same input: gen.exe seed 1, 1,000,000 lines, 820,416 orders
- Load (read + parse), median of 5: 0.316 s
- Matching only, median of 5: 0.843 s (range 0.637 – 1.039)
- ≈ 973,000 orders/sec, ≈ 1.03 µs per order on average

## Per-order latency (Nov 27)
- Same input, 820,416 orders, median of 5 runs
- P50 0.3 us | P99 1.0 us | P99.9 3.4 us | Max ~6.4 ms
- Matching ≈ 1.2M orders/sec (runs vary 0.56–1.02 s)
- Windows clock resolution ~100 ns, so latencies are rounded to 0.1 us
- Max is dominated by rare stalls: OS pauses, plus vector/hash-map growth inside the timed region

## Pre-allocating Trades and mp (Nov 27)
- Change: Trades.reserve() and mp.reserve() before matching
- Median of 5 runs, before → after:
  - P50: 0.3 → 0.3 us
  - P99: 1.0 → 0.9 us
  - P99.9: 3.4 → 2.8 us
  - Max: 6362 → 333 us (~19x lower)
  - Throughput: ~1.20M → ~1.32M orders/sec
- Conclusion: vector growth and hash-map rehashing inside the timed loop caused the ms-level spikes

## Cancel: row search, deep book (Nov 28)
- Input: gen_deep seed 1: 100,000 resting orders on 10 price levels (~10,000 per row), then 100,000 shuffled cancels (all succeed)
- Cancel latency, median of 5: P50 43.9 us | P99 552 us | P99.9 954 us | Max 12.7 ms
- Total matching time: 8.07 s
- On the normal input (big2.txt), cancels were < 0.1 us: rows there are short

## Cancel: direct erase with saved list iterator (Nov 28)
- Change: mp stores {side, price, list iterator}; cancel erases directly instead of searching the row
- Deep book (gen_deep seed 1), cancel latency, median of 5, before → after:
  - P50: 43.9 → 0.5 us (~88x)
  - P99: 552 → 1.1 us (~500x)
  - P99.9: 954 → 2.6 us (~370x)
  - Max: 12.7 ms → 145 us
  - Total matching: 8.07 s → 0.090 s (~90x)
- Complexity: O(log P + k) → O(log P)

