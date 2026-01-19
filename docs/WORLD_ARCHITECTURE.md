# World Architecture: The Relevance Engine

## Overview
The MMOServer uses a **Spatial Hashing** system with **Dynamic Relevance Bubbles**. This replaces the traditional $O(N^2)$ global broadcast with an $O(1)$ constant-time lookup for high-performance entity visibility management.

## Core Mandates (AOP v3.0)

### 1. Contract Supremacy: `ISpatialIndex`
All spatial algorithms must implement the `ISpatialIndex` interface. This allows us to swap the Hashing algorithm for an Octree or Grid without changing the `EntityManager` logic.

### 2. Trace Object: `VisibilityContext`
Every visibility change (Player A sees Player B) generates a `VisibilityContext`. This object carries the TraceId and Timestamp, allowing us to debug desyncs between server state and client render.

### 3. Deterministic Templating
Spatial logic resides in `src/Engine/Spatial`.
- `ISpatialIndex.h`: The Restraining Base.
- `SpatialHash.h/cpp`: The mathematical implementation.
- `InterestManager.h/cpp`: The subscription/AOI logic.

## Technical Specifications

### Spatial Hashing
- **Bucket Size**: 50 units (meters).
- **Lookup Complexity**: $O(1)$.
- **Storage**: Contiguous `std::vector` per bucket for CPU cache optimization.

### Relevance Bubbles (AOI)
- **Inner Circle (High Priority)**: 0-25m. 60Hz updates.
- **Outer Circle (Normal Priority)**: 25-100m. 20Hz updates.
- **Culling Zone**: >150m. Entities are removed from the client's view.

## Threading Model
We utilize a **Reader-Writer Locking** strategy (`std::shared_mutex`).
- **Writers (Simulation)**: Move entities and update bucket assignments.
- **Readers (Networking)**: Query nearby entities to broadcast packets.
- This allows 100% parallel reads during network broadcasts.
