# Define colors
if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColorReset  "${Esc}[m")
    set(ColourBold  "${Esc}[1m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
    set(Blue        "${Esc}[34m")
    set(Magenta     "${Esc}[35m")
    set(Cyan        "${Esc}[36m")
    set(White       "${Esc}[37m")
    set(BoldRed     "${Esc}[1;31m")
    set(BoldGreen   "${Esc}[1;32m")
    set(BoldYellow  "${Esc}[1;33m")
    set(BoldBlue    "${Esc}[1;34m")
    set(BoldMagenta "${Esc}[1;35m")
    set(BoldCyan    "${Esc}[1;36m")
    set(BoldWhite   "${Esc}[1;37m")
endif()

#-----------------------
# Log message functions
#-----------------------

# Print message with not color
function(log text)
    message(STATUS ${text})
endfunction(log text)

# Print debug message
function(log_info text)
    message(STATUS "${Green}${text}${ColorReset}")
endfunction(log_info)

# Print warning message
function(log_warning text)
    message(WARNING "${Yellow}${text}${ColorReset}")
endfunction(log_warning)

# Print error message
function(log_error text)
    message(FATAL_ERROR "${BoldRed}${text}${ColorReset}")
endfunction(log_error)
