# Limit Order Book Matching Engine (C++)

A single-threaded limit order book and matching engine written in C++17. It supports limit orders, market orders and cancels with price-time priority, and it is checked against an independently written brute-force engine on thousands of random inputs.

On my machine, with synthetic data: **about 2 million orders/sec, with P99 latency of 0.9 µs** on a 1M-line workload. Cancelling an order from a 10,000-deep price level takes **0.5 µs (P50), about 88x faster** than the linear search it replaced.

## Features

- Limit orders: trade at the limit price or better; any leftover quantity rests in the book
- Market orders: trade at the best available prices; any leftover quantity is dropped
- Cancels by order id
- Price-time priority: best price first, and at the same price, the order that arrived first
- Partial fills, and orders that sweep through several price levels
- Prices stored as integer ticks (1 tick = 0.01), never as floating point
- Input validation: rejects bad sides, non-positive quantities, negative, zero, malformed or sub-tick prices

## Build and run

Requires g++ with C++17. Commands are for Windows PowerShell.

```powershell
g++ -std=c++17 -O2 -Wall main.cpp -o ob
Get-Content tests\orders.txt | .\ob.exe
```

Run the tests:

```powershell
powershell -ExecutionPolicy Bypass -File .\run_tests.ps1   # fixed test files vs expected outputs
powershell -ExecutionPolicy Bypass -File .\stress.ps1      # 1000 random inputs vs brute-force engine
```

Run the benchmarks:

```powershell
g++ -std=c++17 -O2 -Wall gen.cpp -o gen
g++ -std=c++17 -O2 -Wall gen_deep.cpp -o gen_deep
cmd /c "gen.exe 1 1000000 > big.txt"       # normal workload, 1M lines
cmd /c "gen_deep.exe 1 > deep.txt"         # deep-book cancel workload
cmd /c "ob.exe < big.txt > out.txt"        # timings print to stderr
```

## Input and output format

One command per line:

```
BUY|SELL <quantity> <price>     limit order, e.g. BUY 60 101.00
BUY|SELL <quantity> MKT         market order, e.g. SELL 70 MKT
CANCEL <id>                     cancel a resting order, e.g. CANCEL 3
```

Every order line gets an id (1, 2, 3, ...) in the order it arrives, including rejected ones. Cancel lines do not get an id.

Output: messages for invalid input and failed cancels (in input order), then every trade, then the remaining book. Bids are printed from the highest price, asks from the lowest, and at the same price in arrival order.

Example input:

```
SELL 50 101.00
BUY 40 100.00
SELL 30 102.00
BUY 60 99.00
BUY 60 101.00
SELL 70 MKT
CANCEL 3
CANCEL 9
```

Output:

```
Cancel failed: #9 not found
TRADE buy #5 sell #1 50 @ 101.00
TRADE buy #5 sell #6 10 @ 101.00
TRADE buy #2 sell #6 40 @ 100.00
TRADE buy #4 sell #6 20 @ 99.00
BIDS:
#4 BUY 40 @ 99.00
ASKS:
```

Order #5 buys all 50 from #1 and rests with 10. The market sell #6 then sweeps three price levels: 10 @ 101.00, 40 @ 100.00 and 20 @ 99.00. `CANCEL 3` removes the last ask, and #9 never existed.

## Design

**Price levels: `std::map` from price to the orders at that price.** It keeps prices sorted automatically, so the best price is always `begin()`, and adding a new price costs O(log P), where P is the number of price levels. A sorted vector would have to shift elements on every insert.

**Bids use `std::greater<long long>`.** The best buyer is the one paying the most, but a `std::map` sorts keys from smallest to largest by default. With `greater<>`, the highest bid comes first, so for both sides the best price is `begin()`.

**Orders at one price: `std::list<Order>`.** New orders go to the back with `push_back` and trades take from the front, which gives time priority. The main reason for `list` over `vector`: erasing one element does not move the others, so a saved iterator to an order stays valid while other orders at the same price are filled or cancelled.

**Cancel lookup: `std::unordered_map` from order id to {side, price, list iterator}.** The id identifies an order uniquely, and the map finds it in O(1) on average. The saved iterator lets a cancel erase the order directly instead of searching its price level, so cancel costs O(log P) instead of O(log P + k), where k is the number of orders at that price. Entries are removed as soon as an order leaves the book, so an iterator never points to an erased order.

**Prices as integer ticks.** Binary floating point cannot store most decimals exactly: 0.1 + 0.2 gives 0.30000000000000004. A buyer and a seller at the "same" price could then fail the `buy >= ask` check and not trade, and two equal prices could become two separate map keys. Integer ticks (101.05 → 10105) compare exactly. Prices are parsed from text without ever using `double`.

**Trades happen at the resting order's price.** The resting order was in the book first and publicly stated its price. The incoming order accepted at least that price, so the price improvement goes to the incoming order.

**Market orders** get the price `LLONG_MAX` (buy) or `0` (sell), so the normal price check never stops them; any leftover is dropped instead of resting.

## Testing

1. **Fixed tests.** 10 hand-written inputs in `tests\` (exact fills, partial fills, multi-level sweeps, market orders on an empty book, cancels of resting, filled and unknown orders, invalid input), each with a verified `.expected` output. `run_tests.ps1` compares them automatically.
2. **Brute-force reference engine (`brute.cpp`).** Same input and output format, but the simplest possible method: one vector of all resting orders, scanned in full for every incoming order and cancel. Time priority comes from the smallest id at the same price, since ids increase with arrival. It shares no matching or cancel code with `main.cpp`; otherwise both would share the same bugs.
3. **Random stress test.** `gen.cpp` produces seeded random inputs: prices in a narrow range (99.00 to 101.00) so orders cross and tie often, about 10% market orders, about 18% cancels (half targeting recent orders so they usually succeed), and about 5% invalid lines. `stress.ps1` runs 1000 seeds through both engines and stops at the first difference, saving the failing input.
4. **Checking the stress test can fail.** I planted a bug in the brute-force engine (newest order wins ties instead of oldest) and confirmed the stress test reported a mismatch.
5. **Asserts** check `parse_price` on valid, malformed, sub-tick and overflowing inputs. Parsing is shared by both engines, so the stress test cannot catch parsing bugs.

## Performance

### Setup

- CPU: AMD Ryzen 7 7435HS
- Compiler: g++ 16.2.0 (MSYS2), `-std=c++17 -O2`
- OS: Windows
- Single-threaded

### Method

- The program first loads every command into memory, then times only the matching. File reading and parsing are timed separately.
- Each order and each cancel is timed individually with `std::chrono::steady_clock` to get latency percentiles.
- Every result is the median of 5 runs.
- The Windows clock ticks about every 100 ns, so latencies are rounded to 0.1 µs, and anything faster than one tick shows as 0.
- Timings are printed to stderr, so the test outputs are unaffected.

### Results

**Normal workload:** 1,000,000 lines (820,416 orders, the rest cancels), generated with an earlier version of `gen.cpp` (uniformly random cancel ids, no invalid lines), seed 1

| Measure | Value |
|---|---|
| Load (read and parse) | 0.308 s |
| Matching | 0.399 s |
| Throughput | ~2.05M orders/sec |
| P50 / P99 / P99.9 latency | 0.2 / 0.9 / 6.3 µs |
| Max latency | 473 µs |

**Finding 1: container growth caused millisecond spikes.** At first, the max latency was about 6.4 ms. The cause was the trades vector reallocating and the id hash map rehashing inside the timed loop, so one unlucky order paid for copying everything. Reserving capacity before matching fixed it.

| Measure | Before | After `reserve` |
|---|---|---|
| P99.9 | 3.4 µs | 2.8 µs |
| Max | 6,362 µs | 333 µs (~19x lower) |
| Throughput | ~1.20M/sec | ~1.32M/sec |

**Finding 2: cancel cost depends on how deep the book is.** On the normal workload, cancels took under 0.1 µs because price levels were short. `gen_deep.cpp` models market makers: 100,000 resting orders on 10 price levels (about 10,000 per level), then 100,000 cancels in random order. Here, searching the price level dominated. Storing a list iterator per order removed the search.

| Cancel latency | Linear search | Saved iterator | Speed-up |
|---|---|---|---|
| P50 | 43.9 µs | 0.5 µs | ~88x |
| P99 | 552 µs | 1.1 µs | ~500x |
| P99.9 | 954 µs | 2.6 µs | ~370x |
| Max | 12,656 µs | 145 µs | ~87x |
| Total matching time | 8.07 s | 0.090 s | ~90x |

**Finding 3: the cancel redesign also sped up normal matching.** Before, every incoming order was inserted into the id map and then erased, even if it traded immediately. Now only orders that rest in the book are added. On the normal workload, matching time dropped from 0.623 s to 0.399 s (~1.56x), from ~1.32M to ~2.05M orders/sec. P99.9 rose from 2.8 to 6.3 µs; the cause is not yet investigated.

## Limitations and next steps

- Single-threaded, synthetic random data, one machine. The numbers show relative improvements, not exchange-grade guarantees.
- Clock resolution is about 100 ns, so sub-microsecond latencies are approximate.
- All trades are kept in memory and printed at the end. A real engine would stream them out.
- Capacity is reserved from the known input size. A live system would pre-allocate for expected peak load at startup.
- Known quirks: a non-numeric quantity stops reading input; state is global rather than in a class.
- Next: reuse memory for orders instead of allocating per order (memory pool); a price-indexed array instead of `std::map`; modify orders and IOC orders; split into a class and headers with CMake.