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

