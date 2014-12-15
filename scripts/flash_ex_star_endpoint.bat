START /B msp430-gdbproxy --tcpport=2000 --progport=%1
msp430-gdb --batch -ex="target remote :2000" -ex="load" -ex="quit" ..\oss7-build\examples\star_push_endpoint_example\star_push_endpoint_example 