package require Thread

# Constructor for Linda namespace
namespace eval Linda {}

proc Linda::master_init {} {
    variable thread_pool
    variable current_worker 0
    variable rw_lock
    variable wait_lock
    variable modified_cond
    
    tsv::set ::Linda::shared tuple_space {}
    #set mut_tspace [thread::rwmutex create]
    set rw_lock [thread::rwmutex create]
    set modified_cond [thread::cond create]
    set wait_lock [thread::mutex create]

    # create 10 threads for handling client requests
    set ::Linda::thread_pool {}
    for {set i 0} {$i < 10} {incr i} {
        puts "master spawning worker thread $i"
        lappend thread_pool [thread::create {
            source linda.tcl
            thread::wait
        }]
        # Initialize each thread's copy of the id of the shared mutex
        thread::send [lindex $thread_pool end] "set ::Linda::rw_lock $::Linda::rw_lock"
        thread::send [lindex $thread_pool end] "set ::Linda::modified_cond $::Linda::modified_cond"
        thread::send [lindex $thread_pool end] "set ::Linda::wait_lock $::Linda::wait_lock"
    }
    puts "Master spawned [llength $thread_pool] worker threads."
}

proc Linda::get_next_worker {} {
    variable thread_pool
    variable current_worker

    set worker_id [lindex $thread_pool $current_worker]
    incr current_worker
    if {$current_worker >= [llength $thread_pool]} {
        set current_worker 0
    }
    return $worker_id
}

proc Linda::server {port} {
    variable server_socket

    if {[info exists server_socket]} {
        return
    }

    set server_socket [socket -server Linda::accept $port]
}

proc Linda::accept {sock addr port} {
    puts "Linda: accepted connection from $addr:$port"
    set worker_id [get_next_worker] ;# A function to find the next worker

    fconfigure $sock -buffering line -blocking 0
    flush $sock

    after 0 [list Linda::async_transfer $worker_id $sock]
}

proc Linda::async_transfer {worker_id sock} {
    thread::transfer $worker_id $sock
    thread::send $worker_id [list ::Linda::service $sock]
}

proc Linda::service {sock} {
    fconfigure $sock -buffering line -blocking 0
    fileevent $sock readable [list Linda::read $sock]
}

proc Linda::read {sock} {

    if {[eof $sock]} {
        puts "Linda: connection closed"
        close $sock
        return
    }

    if {[gets $sock line] >= 0} {
        puts "Linda: received command: $line"
        handle_command $sock $line
    }
}

proc Linda::handle_command {sock line} {
    # Placeholder for command handling logic
    # For example, you might parse the command and execute corresponding actions
    puts "Linda: handling command: $line"
    puts "Linda: line received: $line"
    puts "LindaL command received: [lindex $line 0]"
    if {[lindex $line 0] eq "OUT"} {
        set tuple [lindex $line 1]
        out $sock $tuple
    } elseif {[lindex $line 0] eq "RD"} {
        set template [lindex $line 1]
        rd $sock $template
    } else {
        puts "Linda: unknown command: $line"
        puts $sock "Error: unknown command"
        flush $sock
    }
}

proc Linda::verify {tuple {accept_null 0}} {
    set verified_tuple [list]
    foreach pair $tuple {
        set type [lindex $pair 0]
        set val [lindex $pair 1]
     
        if {$type eq "NULL"} {
            if {$val ne "?"} {
                error $sock "Error: NULL type must have value '?'"
            }
            if {!$accept_null} {
                error "Error: NULL value is invalid argument to OUT"
            }
            lappend verified_tuple [list NULL ?]
        } elseif {$type eq "STRING"} {
            lappend verified_tuple [list STRING $val]
        } elseif {$type eq "INT64"} {
            if {![string is integer -strict $val]} {
                error $sock "Error: invalid INT64 value: $val"
            }
            lappend verified_tuple [list INT64 [expr int($val)]]
        } elseif {$type eq "FLOAT64"} {
            if {![string is double -strict $val]} {
                error $sock "Error: invalid FLOAT64 value: $val"
            }
            lappend verified_tuple [list FLOAT64 [expr double($val)]]
        } else {
            error "Error: unknown type: $type"
        }
    }
    return $verified_tuple
}

proc Linda::out {sock tuple} {
    # Note: tuple should be in the form of a Tcl list where
    # each element is a pairt of type and value, e.g., 
    # {STRING "hello"} {INT64 42} {FLOAT64 3.14} {NULL ?}

    variable rw_lock
    variable modified_cond

    puts "OUT $tuple"
    if ![string is list $tuple] {
        puts "Linda: invalid tuple format: $tuple"
        puts $sock "Error: invalid tuple format"
        flush $sock
        return 
    }

    try {
        set verified_tuple [verify $tuple]
    } on error {msg} {
        puts "Linda: $msg"
        puts $sock $msg
        flush $sock
        return
    }

    
    lock_write {
        tsv::lappend ::Linda::shared tuple_space $verified_tuple
        thread::cond notify $modified_cond
    }

    puts "Linda: sending stored tuple: $verified_tuple"
    puts $sock "Stored: $verified_tuple"
    flush $sock
    puts "Linda: current tuple space: [tsv::get ::Linda::shared tuple_space]"
}

proc Linda::lock_write {code} {
    variable rw_lock
    variable readers_cond
    variable writers_cond
    variable current_worker

    thread::rwmutex wlock $rw_lock
    uplevel 1 [list eval $code]
    thread::rwmutex unlock $rw_lock
}
proc Linda::lock_read {code} {
    variable rw_lock

    thread::rwmutex rlock $rw_lock
    uplevel 1 [list eval $code]
    thread::rwmutex unlock $rw_lock
}

proc Linda::wait_for_modified {} {
    variable wait_lock
    variable modified_cond
    thread::mutex lock $wait_lock
    thread::cond wait $modified_cond $wait_lock
    thread::mutex unlock $wait_lock  
}

proc Linda::rd {sock template} {
    puts "Linda: RD command received with template: $template"
    variable rw_lock
    variable modified_cond

    try {
        set template [::Linda::verify $template 1]
    } on error {msg} {
        puts "Linda: $msg"
        puts $sock $msg
        flush $sock
        return
    }   

    set match 0
    while {1} {
        lock_read {
            foreach tuple [tsv::get ::Linda::shared tuple_space] {
                # Simple matching logic (to be improved)
                if {[llength $template] != [llength $tuple]} {
                    continue
                }
                set match 1
                for {set i 0} {$i < [llength $template]} {incr i} {
                    set temp_type [lindex [lindex $template $i] 0]
                    set temp_val [lindex [lindex $template $i] 1]
                    set tup_type [lindex [lindex $tuple $i] 0]
                    set tup_val [lindex [lindex $tuple $i] 1]

                    if {$temp_type ne "NULL" && ($temp_type ne $tup_type || $temp_val ne $tup_val)} {
                        set match 0
                        break
                    }
                }
                if {$match} {
                    break
                }
            }
        }
        if {$match} {
            puts $sock "Found: $tuple"
            flush $sock
            break
        } else {
            #puts $sock "NotFound: No match found"
            puts "Thread waiting for modified tuple space"
            wait_for_modified
            set match 0
        }
    }
}

proc Linda::listen {port} {
    server $port
    puts "Linda: server listening on port $port"
}

proc Linda::shutdown {} {
    puts "To Be Done"
}

proc Linda::status {} {
    variable server_socket
    puts "TO Be Done"
}
