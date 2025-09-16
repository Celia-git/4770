package main

import (
    "encoding/json"
    "fmt"
    "log"
    "math/rand"
    "net"
    "sync"
    "time"
)

type Tuple []interface{}

type Request struct {
    Cmd     string        `json:"cmd"`    // "out", "rd", "in"
    Tuple   Tuple         `json:"tuple,omitempty"`
    Pattern Tuple         `json:"pattern,omitempty"`
}

type Response struct {
    Result Tuple  `json:"result,omitempty"`
    Error  string `json:"error,omitempty"`
}

type TupleSpace struct {
    tuples []Tuple
    mu     sync.Mutex
    cond   *sync.Cond
}

func NewTupleSpace() *TupleSpace {
    ts := &TupleSpace{}
    ts.cond = sync.NewCond(&ts.mu)
    return ts
}

func (ts *TupleSpace) out(tuple Tuple) {
    ts.mu.Lock()
    ts.tuples = append(ts.tuples, tuple)
    ts.cond.Broadcast()
    ts.mu.Unlock()
}

func matchTuple(tuple Tuple, pattern Tuple) bool {
    if len(tuple) != len(pattern) {
        return false
    }
    for i := range pattern {
        if pattern[i] == nil || pattern[i] == "?" {
            continue // wildcard
        }
        if tuple[i] != pattern[i] {
            return false
        }
    }
    return true
}

func (ts *TupleSpace) findMatchIdx(pattern Tuple) (int, bool) {
    for idx, t := range ts.tuples {
        if matchTuple(t, pattern) {
            return idx, true
        }
    }
    return -1, false
}

func (ts *TupleSpace) findAllMatchIdx(pattern Tuple) []int {
    idxs := []int{}
    for idx, t := range ts.tuples {
        if matchTuple(t, pattern) {
            idxs = append(idxs, idx)
        }
    }
    return idxs
}

func (ts *TupleSpace) in(pattern Tuple) Tuple {
    ts.mu.Lock()
    defer ts.mu.Unlock()
    for {
        idxs := ts.findAllMatchIdx(pattern)
        if len(idxs) > 0 {
            pick := idxs[rand.Intn(len(idxs))]
            tuple := ts.tuples[pick]
            // Remove tuple
            ts.tuples = append(ts.tuples[:pick], ts.tuples[pick+1:]...)
            return tuple
        }
        ts.cond.Wait()
    }
}

func (ts *TupleSpace) rd(pattern Tuple) Tuple {
    ts.mu.Lock()
    defer ts.mu.Unlock()
    for {
        idxs := ts.findAllMatchIdx(pattern)
        if len(idxs) > 0 {
            pick := idxs[rand.Intn(len(idxs))]
            tuple := ts.tuples[pick]
            return tuple
        }
        ts.cond.Wait()
    }
}

// --- Server code

func handleConn(conn net.Conn, ts *TupleSpace) {
    defer conn.Close()
    decoder := json.NewDecoder(conn)
    encoder := json.NewEncoder(conn)

    for {
        var req Request
        if err := decoder.Decode(&req); err != nil {
            return
        }
        switch req.Cmd {
        case "out":
            ts.out(req.Tuple)
            encoder.Encode(Response{})
        case "rd":
            tuple := ts.rd(req.Pattern)
            encoder.Encode(Response{Result: tuple})
        case "in":
            tuple := ts.in(req.Pattern)
            encoder.Encode(Response{Result: tuple})
        default:
            encoder.Encode(Response{Error: "unknown command"})
        }
    }
}

func main() {
    ln, err := net.Listen("tcp", ":8080")
    if err != nil {
        log.Fatal(err)
    }
    fmt.Println("Tuple space server started on :8080")
    rand.Seed(time.Now().UnixNano())
    ts := NewTupleSpace()
    for {
        conn, err := ln.Accept()
        if err != nil {
            continue
        }
        go handleConn(conn, ts)
    }
}

