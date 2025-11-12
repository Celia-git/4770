***

### Session 1: Threads and Processes

**Focus Explanations:**

- **Threads vs. Processes: Roles and Characteristics**
  A process is an independent program execution instance with its own memory space, system resources, and execution context. It represents a program in action. In contrast, a thread is the smallest unit of execution within a process. Threads share the same memory and resources of their parent process but have their own stack and program counter. This makes threads lightweight compared to processes, enabling concurrent execution within a single process.

- **Context Switching Cost Differences**
  Process context switching involves saving and loading the entire process state including memory maps and resources, which incurs higher overhead. Thread switching is cheaper because threads share most of their context within a process; only thread-specific state must be saved/restored, making thread switching faster and more efficient.

- **Threading Models: User-Level, Kernel-Level, Hybrid**
  User-level threads are managed by a user-space library and invisible to the OS kernel, allowing very fast thread operations but causing blocking issues since the kernel treats them as a single thread. Kernel-level threads are managed by the OS, scheduled independently with higher overhead but better concurrency and blocking behavior. Hybrid models combine both, often mapping multiple user threads onto kernel threads for balanced performance.

- **Multithreading Goals in Clients and Servers**
  Multithreading allows clients and servers to hide network latency by continuing local work while waiting for responses, issue parallel requests simultaneously to multiple resources, and improve overall responsiveness, especially in GUIs and servers. For example, a browser fetching multiple images/scripts in parallel or a server using dispatcher/worker models to handle multiple client requests concurrently.

**Practice Quiz Answers:**

1. Threads are lightweight execution units within a process sharing memory, while processes are independent execution environments with separate memory.
2. Kernel-level threads are individually scheduled by the OS, allowing true concurrency and better handling of blocking I/O, despite higher overhead compared to user-level threads.
3. Multithreading improves responsiveness by enabling parallel request handling and hiding network latency, so clients/servers can process other tasks while waiting.

***

### Session 2: Virtualization and Server Design

**Focus Explanations:**

- **Virtualization Purpose and Characteristics**
  Virtualization abstracts hardware to provide portability (running the same system on different hardware), isolation (separating environments for security and stability), and migration (moving virtual machines freely). This enables flexible resource use and system management.

- **Types of Virtual Machine Monitors (VMM)**
  Process VMs run as applications providing a virtualized environment. Native VMMs run directly on physical hardware to manage multiple OS instances with minimal overhead. Hosted VMMs run on a host OS, leveraging its services, typically with higher overhead. Each has trade-offs between performance and ease of use.

- **Paravirtualization and Containers**
  Paravirtualization involves modifying guest OS to interact efficiently with the hypervisor for better performance. Containers virtualize at the OS level, sharing the kernel but isolating applications via namespaces and cgroups, offering lightweight, fast deployment but less isolation than full VMs.

- **Stateful vs. Stateless Servers**
  Stateful servers maintain client session data between requests, offering rich interactions but with higher complexity and potential performance costs. Stateless servers treat each request independently, enhancing scalability and simplicity at the expense of session awareness.

- **Out-of-Band Communication**
  Methods like separate ports or transport-layer support are used for control messages outside normal data channels, improving responsiveness and modularity.

**Practice Quiz Answers:**

1. Paravirtualization modifies the guest OS for better communication with the host, unlike full virtualization which emulates hardware completely.
2. Stateful servers store session info and context, while stateless servers process each request independently, affecting scalability and reliability.
3. Out-of-band communication can be implemented via separate network ports or through dedicated transport-layer protocols for control signaling.

***

### Session 3: Client-Server Interaction and Object Servers

**Focus Explanations:**

- **X Window System and Virtualization Challenges**
  The X Window System provides a network-transparent windowing service enabling GUI display over networks. Its virtualization and distribution transparency suffer from performance issues such as latency and bandwidth limits. VNC mitigates these by sending compressed screen updates instead of individual drawing commands.

- **Stateful vs. Stateless Objects**
  Stateful objects maintain data across invocations, affecting future interactions, while stateless objects treat each request fresh without persistence. This distinction impacts scalability and concurrency.

- **Threading Models in Object Servers**
  Single-threaded handles requests sequentially. Per-thread dedicates a thread per request, enabling concurrency. Thread pools use a fixed set of threads to balance resource use and throughput.

**Practice Quiz Answers:**

1. The X Window System enables GUI over networks but can perform poorly due to high latency; VNC reduces this by transmitting image diffs.
2. Stateful objects keep data between calls; stateless objects do not, increasing ease of scaling.
3. Thread pools efficiently handle multiple concurrent requests by managing a fixed number of threads.

***

### Session 4: Code Migration and Communication Models

**Focus Explanations:**

- **Code Migration and Mobility Models**
  Code migration involves moving code and sometimes execution state between machines for load balancing or privacy. Weak mobility moves code and data; strong mobility also transfers the execution state allowing migration mid-execution.

- **VM Migration Techniques**
  Push migrates VM state proactively; stop-and-copy halts VM to transfer state causing downtime; pull-on-demand fetches needed state dynamically. Performance varies mainly by downtime and network overhead.

- **OSI Layers and Transport Layer Roles**
  OSI model has 7 layers: Physical (bits), Data Link (frames), Network (routing), Transport (reliable/unreliable delivery), Session, Presentation, and Application. Transport ensures end-to-end reliability (TCP) or fast unreliable delivery (UDP).

- **Middleware and Communication Types**
  Middleware provides services like naming and security. Communication can be transient or persistent (message storage), synchronous or asynchronous (blocking or non-blocking client behavior).

- **Drawbacks of Transient Synchronous Communication**
  Clients and servers must be active simultaneously, causing blocking delays and immediate failure handling, unsuitable for some apps.

**Practice Quiz Answers:**

1. Weak mobility moves code/data only; strong mobility also moves the running execution state.
2. The OSI layers are Physical, Data Link, Network, Transport, Session, Presentation, Application. The transport layer ensures reliable (TCP) or unreliable (UDP) message delivery.
3. Transient synchronous communication causes blocking and requires active client-server presence, limiting use cases.

***

### Session 5: RPC and Socket Programming

**Focus Explanations:**

- **RPC Basics and Flow**
  RPC allows a program to call procedures on remote systems. It involves stub generation (client/server proxies), marshalling parameters into byte streams, OS-level message transport, and unmarshalling results.

- **Parameter Passing and Marshaling**
  Parameters are copied in/out with marshaling converting data to agreed byte formats to ensure interoperability across heterogeneous systems.

- **Access Transparency Limitations**
  RPC hides network details but struggles with remote references and asynchronous needs, leading to added RPC versions (async, multicast) for extended flexibility.

- **Berkeley Socket API and ZeroMQ Patterns**
  Berkeley sockets provide network communication primitives: socket creation, binding, listening, accepting, connecting, sending, receiving, and closing. ZeroMQ abstracts sockets further with patterns like request-reply (synchronous), publish-subscribe (broadcast), and pipeline (streaming) for scalable messaging.

**Practice Quiz Answers:**

1. RPC flow: stub creation, message marshaling, message transport via OS, unmarshaling, procedure execution, and returning results.
2. Marshaling converts complex data structures into byte streams for transport, assuring data integrity across different systems.
3. Request-Reply supports synchronous communication; Publish-Subscribe enables message broadcasting; Pipeline supports asynchronous streaming.

***
