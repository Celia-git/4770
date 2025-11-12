***

### Session 1: Threads and Processes (25 mins)

**Focus:**
- Understand threads vs processes: roles, characteristics, difference in context switching cost.
- Threading models: user-level, kernel-level, hybrid; pros/cons especially for performance.
- Multithreading in clients/servers: goals like hiding network latency, improving responsiveness.
- Examples: Parallel HTTP requests, RPC parallelism, dispatcher/worker models.

**Practice quiz:**
1. Define the main differences between a thread and a process.
2. What are the advantages of kernel-level threads over user-level threads?
3. How does multithreading improve client-server interaction responsiveness?

***

### Session 2: Virtualization and Server Design (30 mins)

**Focus:**
- Purpose and characteristics of virtualization: portability, isolation, migration.
- Types of virtual machine monitors (VMM): process VM, native VMM, hosted VMM.
- Paravirtualization and containers.
- Server design: stateful vs stateless servers, performance/reliability tradeoffs.
- Out-of-band communication methods.
- Overview of common services and port assignments.

**Practice quiz:**
1. What is paravirtualization and how does it differ from full virtualization?
2. Contrast stateful and stateless servers with performance implications.
3. Describe two methods of out-of-band communication in server design.

***

### Session 3: Client-Server Interaction and Object Servers (30 mins)

**Focus:**
- X Window System and its virtualization and distribution transparency problems.
- Web browser virtualization and how VNC addresses performance issues.
- Object servers: managing distributed objects with data and behavior.
- Threading models for distributed object activation (single-threaded, per-thread, thread pool).
- Object state: stateful vs stateless.

**Practice quiz:**
1. What is the X Window System and what challenges arise with virtualization in it?
2. Explain the difference between stateful and stateless objects.
3. What threading model in object servers supports high concurrency?

***

### Session 4: Code Migration and Communication Models (25 mins)

**Focus:**
- Code migration: definitions, usability for load balancing, privacy, flexibility.
- Mobility models: weak (code + data), strong (including execution).
- VM migration techniques: push, stop-and-copy, pull-on-demand; focus on downtime issues.
- Layered communication models: OSI model layers and functions.
- Low-level layers: physical, data link, network.
- Transport layer: TCP vs UDP.
- Middleware purpose and communication types: transient/persistent, synchronous/asynchronous.
- Client-server synchronization points and transient synchronous communication drawbacks.

**Practice quiz:**
1. Explain the difference between weak and strong mobility models.
2. Name the seven OSI layers and the primary role of the transport layer.
3. What are the drawbacks of transient synchronous communication?

***

### Session 5: RPC and Socket Programming (20 mins)

**Focus:**
- Remote Procedure Call (RPC) basics: purpose, flow (stub creation, message packing).
- Parameter passing methods and marshaling.
- Limitations of access transparency in RPC.
- Asynchronous and multicast RPC extensions.
- Berkeley Socket API functions and Python socket programming examples.
- ZeroMQ patterns: Request-Reply, Publish-Subscribe, Pipeline.

**Practice quiz:**
1. What are the key steps in a typical RPC flow?
2. How does marshaling support RPC communication?
3. Name and briefly describe the three ZeroMQ socket patterns.

***
