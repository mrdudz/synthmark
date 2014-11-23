

SuperCPU optimization options:

Optimization    SuperCPU v1             SuperCPU v2
Default         Mirror $0000-$ffff      Mirror $0200-$ffff
None            Mirror $0000-$ffff      Mirror $0000-$ffff
Full            Mirror $0400-$07ff      Mirror no memory

--------------------------------------------------------------------------------

TODO:
- make individual tests run longer, so the results will not jump around as much
  as they do now (CAUTION: this will invalidate all older test results :/)
- pause for a second or two after each test when display is disabled so the
  screen can be read more easily
- add chameleon specific optimization options, eg disable framebuffer writes
- perhaps add 65816 specific tests from amidog -> http://scpu.amidog.se/doku.php?id=scpu:synthmark816