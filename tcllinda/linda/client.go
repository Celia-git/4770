package main

import (
    "encoding/json"
    "flag"
    "fmt"
    "net"
    "os"
    "strconv"
    "strings"
)

// Parse tuple arguments into a slice of interface{}
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
            i, err := strconv.ParseInt(val, 10, 64)
            if err != nil {
                fmt.Println("Error parsing INT64:", err)
                os.Exit(1)
            }
            tuple = append(tuple, i)
        case "FLOAT64":
            f, err := strconv.ParseFloat(val, 64)
            if err != nil {
                fmt.Println("Error parsing FLOAT64:", err)
                os.Exit(1)
            }
            tuple = append(tuple, f)
        default:
            tuple = append(tuple, val)
        }
    }
    return tuple
}

// Print operation and tuple values in natural language
func describeTuple(op string, tuple []interface{}) {
    desc := ""
    for i, v := range tuple {
        if i > 0 {
            desc += " and "
        }
        switch val := v.(type) {
        case string:
            if val == "?" {
                desc += "wildcard"
            } else {
                desc += fmt.Sprintf("string \"%v\"", val)
            }
        case int64:
            desc += fmt.Sprintf("int64 %d", val)
        case float64:
            desc += fmt.Sprintf("float64 %v", val)
        case nil:
            desc += "wildcard"
        default:
            desc += fmt.Sprintf("%T %v", v, v)
        }
    }
    switch op {
    case "out":
        fmt.Printf("Tuple with %s stored in tuple space\n", desc)
    case "in":
        fmt.Printf("Tuple with %s deleted (consumed) from tuple space\n", desc)
    case "rd":
        fmt.Printf("Tuple with %s read (but not removed) from tuple space\n", desc)
    }
}

func main() {
    host := flag.String("host", "localhost:8080", "server address")

    outFlag := flag.String("out", "", "Tuple to insert, e.g. 'STRING:foo INT64:42'")
    inFlag := flag.String("in", "", "Tuple template for deletion, e.g. 'STRING:foo ?'")
    rdFlag := flag.String("rd", "", "Tuple template for read, e.g. 'STRING:foo ?'")

    flag.Parse()

    conn, err := net.Dial("tcp", *host)
    if err != nil {
        fmt.Println("Unable to connect:", err)
        os.Exit(1)
    }
    defer conn.Close()

    encoder := json.NewEncoder(conn)
    decoder := json.NewDecoder(conn)

    var req map[string]interface{}
    var tuple []interface{}
    var op string

    if *outFlag != "" {
        tuple = parseTuple(strings.Fields(*outFlag))
        req = map[string]interface{}{"cmd": "out", "tuple": tuple}
        op = "out"
    } else if *inFlag != "" {
        tuple = parseTuple(strings.Fields(*inFlag))
        for i, v := range tuple {
            if s, ok := v.(string); ok && s == "?" {
                tuple[i] = nil
            }
        }
        req = map[string]interface{}{"cmd": "in", "pattern": tuple}
        op = "in"
    } else if *rdFlag != "" {
        tuple = parseTuple(strings.Fields(*rdFlag))
        for i, v := range tuple {
            if s, ok := v.(string); ok && s == "?" {
                tuple[i] = nil
            }
        }
        req = map[string]interface{}{"cmd": "rd", "pattern": tuple}
        op = "rd"
    } else {
        fmt.Println("Error: must specify one of -out, -in, or -rd")
        os.Exit(1)
    }

    describeTuple(op, tuple)

    if err := encoder.Encode(req); err != nil {
        fmt.Println("Failed to send request:", err)
        os.Exit(1)
    }

    var resp map[string]interface{}
    if err := decoder.Decode(&resp); err != nil {
        fmt.Println("Failed to read response:", err)
        os.Exit(1)
    }

    if errStr, ok := resp["error"]; ok && errStr != nil {
        fmt.Println("Error:", errStr)
    } else if result, ok := resp["result"]; ok && result != nil {
        fmt.Println("Result:", result)
    } else {
        fmt.Println("OK")
    }
}
