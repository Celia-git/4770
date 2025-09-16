package main

import (
    "bufio"
    "encoding/json"
    "flag"
    "fmt"
    "net"
    "os"
    "strconv"
    "strings"
)

// Input: cmd args..., e.g.:
// out STRING:Hello INT64:35
// rd STRING:? INT64:35

func parseTuple(args []string) []interface{} {
    tuple := make([]interface{}, 0, len(args))
    for _, arg := range args {
        if arg == "?" {
            tuple = append(tuple, "?")
            continue
        }
        parts := strings.SplitN(arg, ":", 2)
        if len(parts) != 2 {
            tuple = append(tuple, arg)
            continue
        }
        typ, val := parts[0], parts[1]
        switch typ {
        case "STRING":
            tuple = append(tuple, val)
        case "INT64":
            i, _ := strconv.ParseInt(val, 10, 64)
            tuple = append(tuple, i)
        case "FLOAT64":
            f, _ := strconv.ParseFloat(val, 64)
            tuple = append(tuple, f)
        default:
            tuple = append(tuple, val)
        }
    }
    return tuple
}

func main() {
    host := flag.String("host", "localhost:8080", "server address")
    flag.Parse()

    conn, err := net.Dial("tcp", *host)
    if err != nil {
        fmt.Println("Unable to connect:", err)
        os.Exit(1)
    }
    defer conn.Close()
    decoder := json.NewDecoder(conn)
    encoder := json.NewEncoder(conn)
    reader := bufio.NewReader(os.Stdin)

    fmt.Println("Linda Client. Commands: out ..., rd ..., in ...")
    for {
        fmt.Print("> ")
        line, err := reader.ReadString('\n')
        if err != nil {
            break
        }
        line = strings.TrimSpace(line)
        if line == "" {
            continue
        }
        fields := strings.Fields(line)
        cmd := fields[0]
        tuple := parseTuple(fields[1:])
        var req map[string]interface{}
        switch cmd {
        case "out":
            req = map[string]interface{}{"cmd": "out", "tuple": tuple}
        case "rd":
            req = map[string]interface{}{"cmd": "rd", "pattern": tuple}
        case "in":
            req = map[string]interface{}{"cmd": "in", "pattern": tuple}
        default:
            fmt.Println("Unknown command")
            continue
        }
        // Send request
        encoder.Encode(req)
        // Receive response
        var resp map[string]interface{}
        decoder.Decode(&resp)
        if errStr, ok := resp["error"]; ok && errStr != nil {
            fmt.Println("Error:", errStr)
        } else if result, ok := resp["result"]; ok && result != nil {
            fmt.Println("Result:", result)
        } else {
            fmt.Println("OK")
        }
    }
}

