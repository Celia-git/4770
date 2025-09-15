source linda.tcl

proc main {} {
    Linda::master_init
    Linda::listen 12345
    # The server will run indefinitely; you can add a mechanism to stop it if needed
    vwait forever
}

main