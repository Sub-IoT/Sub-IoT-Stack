set remote hardware-breakpoint-limit 6
set remote hardware-watchpoint-limit 4

define hook-step
mon cortex_m maskisr on
end
define hookpost-step
mon cortex_m maskisr off
end