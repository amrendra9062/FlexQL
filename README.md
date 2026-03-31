# FlexQL: High-Performance Persistent Database Engine

FlexQL is a custom-engineered, high-performance **Relational Database Management System (RDBMS)**.  
It is designed as a hybrid system that combines the extreme speed of **in-memory storage** with the reliability of **disk persistence**.

**Key Achievement:** Achieved a peak throughput of **270,124 rows/sec** during the 1,000,000 row insertion benchmark.

---

### What is FlexQL

FlexQL is an in-memory database management system (IMDBMS) developed to support high-throughput analytical queries.  
By utilizing a **row-major storage model**, the engine ensures sub-millisecond query response times.

Unlike standard volatile systems, FlexQL implements a **Write-Ahead Log (WAL)** to ensure all data is stored persistently and the system remains fault-tolerant.

---

## Implementation Overview

- **Storage Model:**  
  Uses a row-major format (`std::vector<Row>`) to optimize CPU cache locality during full table scans.

- **Indexing:**  
  Implements a **B+ Tree** to provide `O(log N)` search complexity for both point lookups and range queries.

- **Persistence (WAL):**  
  Every mutation (`CREATE`, `INSERT`, `DELETE`) is logged to `flexql.wal` before being committed to RAM, allowing full recovery after crashes.

- **Concurrency:**  
  Uses a **thread-per-connection** model with `std::mutex` to safely handle multiple clients.

- **Batch Insertion:**  
  Supports large SQL payloads in a single network round-trip, significantly improving throughput.

- **Zero-Copy Pipeline:**  
  Processes query results using pointers to minimize memory overhead and accelerate operations like sorting.

---

## How to Run FlexQL

### 1. Prerequisites

Ensure you are in a Linux environment with:
- `g++` (C++17 support)
- `make`

---

### 2. Compilation and Execution

```bash
make clean
make all

Starting the Server
The server must be running to handle requests. It will automatically recover data from flexql.wal if it exists.

./flexql_server


Running Benchmarks
Run performance tests and unit tests:

./run_benchmark

Interactive Testing
Use the client to execute SQL queries manually:

./flexql_client

Example:

SELECT * FROM TEST_USERS WHERE ID > 10;