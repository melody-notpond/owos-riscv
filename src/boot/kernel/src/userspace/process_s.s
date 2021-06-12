.section .text

.global process_switch_context

# process_switch_context(pid_t) -> void
# Switches the current context to a process and continues execution of that process.
#
# Parameters:
# a0: pid_t     - The pid of the process to switch to.
# Returns: nothing
# Modifies all registers.
process_switch_context:
    mret
