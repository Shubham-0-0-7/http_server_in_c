# minimal http/1.0 server in c

this project is a minimal http/1.0 server written in c with focus on **correctness, security boundaries, and memory safety**, not features or performance. the goal was to understand how http actually works on top of tcp, and how real trust boundaries are enforced at the byte level.

this is not a production server and does not aim to be one. it is a learning project meant to build a correct mental model of tcp, http parsing, and defensive server design.

---

## motivation

most “http server from scratch” projects jump directly to features or frameworks and hide the important parts:
- tcp being a byte stream
- protocol framing
- server-side validation
- memory ownership and lifetimes

this project intentionally stays minimal and explicit so every decision is understandable and defensible.

learning inspiration came from:
- lion’s http server series on youtube  
- linux man pages  
- beej’s guide to network programming  
- rfc 1945 (http/1.0 specification)

---

## what this server does

- listens on a tcp socket
- reads incoming data as a stream (not assuming request boundaries)
- detects http framing using `\r\n\r\n`
- parses only the request line (method, uri, version)
- performs strict server-side uri validation
- routes requests using explicit comparisons
- generates http responses according to the spec
- closes the connection (http/1.0 semantics)

---

## what this server intentionally does NOT do

this is a deliberate design choice.

- no http/1.1 keep-alive
- no chunked transfer encoding
- no threading or async
- no tls
- no full header parsing
- no dynamic memory allocation in request path

the goal is correctness and clarity, not completeness.

---

## request lifecycle (mental model)

a request in this server is **not an object**, it is **raw bytes over tcp**.

1. `accept()` returns a connected socket, not an http request  
2. bytes are read incrementally into a fixed-size buffer  
3. parsing does not begin until `\r\n\r\n` is detected  
4. the request line is extracted and parsed syntactically  
5. the uri is validated server-side (security boundary)  
6. optional normalization may be applied to already valid paths  
7. routing is performed using simple string comparison  
8. the response is constructed explicitly (status line + headers + body)  
9. response is written to the socket and the connection is closed  

this separation between transport, parsing, validation, and routing is intentional.

---

## tcp is a stream

tcp does not preserve message boundaries.

- one `read()` does not equal one request
- headers can arrive split across reads
- multiple requests could arrive in one read

because of this, the server:
- accumulates data in a buffer
- never assumes a full request is present
- enforces protocol framing explicitly

---

## parsing vs validation vs normalization

these are separate concerns and are kept separate in the code.

- **parsing**: is the request line syntactically valid?
- **validation**: is the request allowed?
- **normalization**: what is the canonical form of a valid path?

malicious input is rejected, not rewritten.

example:
`/../hello -> rejected`


the server does not normalize traversal attempts.

---

## server-side validation (security boundary)

clients cannot be trusted.

browsers normalize paths before sending requests, attackers do not. therefore all validation is done server-side on raw input.

uri validation rules:
- must start with `/`
- must not contain `..`
- must be within bounded length

validation happens **before routing or filesystem access**.

this is the main security boundary of the server.

---

## memory safety by design

memory safety is achieved by structure, not by tools.

key decisions:
- fixed-size stack buffers
- explicit length tracking
- zero-copy string views (`pointer + length`)
- no heap allocation in the request path
- no recursion
- deterministic ownership and lifetimes

this eliminates:
- buffer overflows
- use-after-free
- memory leaks
- allocator failure paths

stack usage is bounded and predictable.

---

## testing approach

testing was done against raw input, not browser behavior.

tools used:
- `curl --path-as-is`
- `nc` (netcat)
- malformed request lines
- oversized headers

this ensures the server behaves correctly under adversarial input.

---

## limitations

this project is intentionally limited.

- only handles simple GET-style requests
- no persistent connections
- no concurrency
- no filesystem serving (yet)

these limitations are accepted in exchange for clarity and correctness.

---

## how to run

```bash
gcc server.c -o server
./server
```

then test with  
```bash
curl http://localhost:1337/hello
curl --path-as-is http://localhost:1337/../hello
```

### final note
this project is not about saying “i built an http server”.  
it is about understanding:
- how tcp and http interact
- where trust boundaries exist
- how to write defensive, memory-safe systems code in c

features can always be added later. correctness comes first.
