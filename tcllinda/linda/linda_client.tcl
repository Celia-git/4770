proc linda_client_connect {host port} {

    set linda_client_socket [socket $host $port]
    fconfigure $linda_client_socket -buffering line 
    puts "Linda: connected to server at $host:$port"
    return $linda_client_socket
}

proc linda_client_send {csocket msg} {
    puts "calling send..."
    puts $csocket $msg
    flush $csocket
    puts "Linda: sent message: $msg"
}

proc comment {code} {
}

# run it like so:
# tclsh linda_client.tcl -OUT "{{STRING {Hello}} {INT64 42} {FLOAT64 3.14}}"
# tclsh linda_client.tcl -RD "{{NONE ?} {NONE ?} {FLOAT64 3.14}}"
proc main {} {
    # Example usage
    set host "localhost"
    set port 12345

    set csocket [linda_client_connect $host $port]
    if {[info exists csocket]} {

        #linda_client_send $csocket "OUT {{STRING {Hello}} {INT64 42} {FLOAT64 3.14}}"
        #chan gets $csocket line_data
        #puts "Linda: received response: $line_data"
        #linda_client_send $csocket "RD {{STRING {Hello}} {INT64 42} {FLOAT64 3.14}}"
        #chan gets $csocket line_data
        #puts "Linda: received response: $line_data"
        #close $csocket
        set op [lindex $::argv 0]
        switch $op {
            "-OUT" {
                set data [lindex $::argv 1]
                linda_client_send $csocket "OUT $data"
            }
            "-IN" {
                set template [lindex $::argv 1]
                linda_client_send $csocket "IN $template"
                chan gets $csocket line_data
                puts "Linda: received response: $line_data"
            }
            "-RD" {
                set template [lindex $::argv 1]
                linda_client_send $csocket "RD $template"
                chan gets $csocket line_data
                puts "Linda: received response: $line_data"
            }
            default {
                puts "Unknown operation: $op"
            }
        }
    }
}

main