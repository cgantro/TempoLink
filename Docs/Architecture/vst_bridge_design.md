# Architecture: TempoLink VST3 Bridge Design (Syncroom Style)

This document formalizes the architectural design for the future VST3 Bridge, based on the research and insights gathered from the SYNCROOM VST mode implementation.

## 1. Core Philosophy: The Audio Pipe
The VST3 plugin is **feature-less**. It does not handle networking, session logic, or complex audio processing. Its sole purpose is to act as a low-latency bidirectional audio conduit between the DAW and the **TempoLink Standalone App**.

### Data Flow Diagram
```text
[ DAW Environment ]
       │
       ▼ (Audio Input)
[ TempoLink VST3 Bridge ] ◄────┐
       │                       │
       ▼ (Shared Memory IPC)   │ (Low Latency Loop)
       │                       │
[ TempoLink Standalone App ] ──┘
       │ (Brain: UI, Session, Mix)
       ▼
 [ Remote Network Peers ]
```

## 2. Technical Implementation Requirements

### A. Inter-Process Communication (IPC)
- **Mechanism**: Shared Memory (Memory Mapped Files).
- **Data Structure**: Lock-free Circular Buffers (Ring Buffers) to ensure no blocking on high-priority audio threads.
- **Synchronization**: Use lightweight atomic flags; avoid mutexes or system calls in the VST `processBlock()`.

### B. Master-Slave Relationship
- **Master (App)**: Owns the session, handling all UDP/WebSocket networking, UI interactions, and global audio mixing.
- **Slave (Plugin)**: Only responsible for copying audio buffers from the DAW to the IPC layer and vice versa.

### C. Auto-Discovery & Launch
- When the VST3 plugin is initialized in a DAW, it should verify if the Standalone App is running.
- If not running, it may optionally signal the system to launch the Standalone App.

## 3. Implementation Roadmap
1. **Phase 1 (Current)**: Focus on completing the Standalone UI and Audio Engine.
2. **Phase 2**: Stabilize the `RealtimeCore` shared networking library.
3. **Phase 3**: Develop the `RealtimeCore::IPC` layer for memory-mapped audio sharing.
4. **Phase 4**: Implement the VST3 Bridge using the "Audio Pipe" logic.

---
*Created on 2026-04-01 based on user-driven architectural research.*
