[33mcommit 3b3e27146ed7d666b35acf63ebeb58bb7537ea74[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Sat May 4 08:42:53 2013 +0200

    fixing subnet specifier

[33mcommit 9073628703e37cffbe04458cf12077606d895930[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu May 2 12:16:36 2013 +0200

    working on advertising protocol

[33mcommit 7fe6ff3a088e97328024beab9f019c795fd08e1b[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu May 2 11:58:51 2013 +0200

    fixing timer

[33mcommit e760a4b29d8eb237cad78f5501257c6460866d4c[m
Merge: 6b9c020 be044cd
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu May 2 11:46:49 2013 +0200

    Merge branch 'master' into macimplementation

[33mcommit be044cddb3f3887cfdf074ef0962d97b96846f5e[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu May 2 11:46:12 2013 +0200

    substracting elapsed time from timer event

[33mcommit 6b9c02074bc0ec7d961478ec37d1b4ce4924ae81[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu May 2 11:19:16 2013 +0200

    sync traing work in progress

[33mcommit 3ca866ff4a9d088823ec4516e502fbc14b952e2d[m
Merge: c9aae37 05ff5f2
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu May 2 10:13:26 2013 +0200

    merged timer fix in macimplementation

[33mcommit 05ff5f2dd16ef93157ecb56dc8d3302549b5987e[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu May 2 10:07:14 2013 +0200

    timer queue event now inserted on right place, TODO: elapsed time should be substracted

[33mcommit 883a6f712d80e7e59b0aec61d47f1321cd84c1dd[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu May 2 08:42:38 2013 +0200

    fixing timer queue order

[33mcommit c9aae37b936586a25cce8051b1b2429c077b875f[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed May 1 22:37:07 2013 +0200

    merged master

[33mcommit f42f41bb07758a9f7f7d5240623570ae467079f7[m
Merge: 8058afe 24d1b59
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed May 1 22:36:17 2013 +0200

    Merge branch 'master' into macimplementation

[33mcommit 8058afeb2410f90d36b885a4d3ea761949dc5fa8[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed May 1 22:31:39 2013 +0200

    implementing synctrain

[33mcommit 24d1b59aaf388a00e5fc9479e2d324efbf09ebb8[m
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Wed May 1 15:41:09 2013 +0200

    Changed type of variables

[33mcommit 54f6d2a7a79373da7c065e0a2b7594dab2090f10[m
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Wed May 1 15:15:19 2013 +0200

    Fixed bug and typo

[33mcommit 801b77118842f68ce6f020a7712271ca6fa3db66[m
Author: Tom Coopman <tom.coopman@student.artesis.be>
Date:   Wed May 1 15:12:15 2013 +0200

    Forgot a to uncommend a function that gives problems on my side. See isues for more information

[33mcommit 5ffb61a7d9f7e5ac5a7445613b19c476e0e327ab[m
Author: Tom Coopman <tom.coopman@student.artesis.be>
Date:   Wed May 1 14:54:47 2013 +0200

    gave value int8 to arrays for EIRP

[33mcommit 890a84af630f4ad07c37772be735afbc345e7204[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed May 1 09:37:29 2013 +0200

    implementing synctrain

[33mcommit bab207797f31583291a0d074f45a034e34432dd1[m
Merge: 2639e08 a833f79
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed May 1 08:38:29 2013 +0200

    merged master into macimplementations

[33mcommit a833f7973e142e63b984c32af0f2a4dfa2bddfd4[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu Apr 18 09:25:22 2013 +0200

    increasing dialog_id

[33mcommit ebb412f9a540872c0776369b85ad70b5d939fd38[m
Author: Tom Coopman <tom.coopman@student.artesis.be>
Date:   Mon Apr 8 15:42:24 2013 +0200

    fault in array for EIRP

[33mcommit 96ddc62615c61ff3fe8bc0d19556dd77579047df[m
Author: Tom Coopman <tom.coopman@student.artesis.be>
Date:   Mon Apr 8 00:43:16 2013 +0200

    implemented EIRP

[33mcommit cddedba14df006c0071a207c850ea16a72091c57[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed Mar 20 18:21:01 2013 +0100

    implented watchdogtimer

[33mcommit f1e0200a5454476ca60f49d9c9f67d11f8923b09[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed Mar 20 13:15:31 2013 +0100

    fixed some deadlock, but still problem in rs_data_isr when a lot of data is comming in

[33mcommit c034fcc8324f8291552df7e50c805479df074f55[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Mon Mar 18 16:26:53 2013 +0100

    fixed bug, no rx continue after crc error, added todo in phy

[33mcommit 37927262885077032eddbd4123576b4165f528e3[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Fri Mar 15 13:28:18 2013 +0100

    fixing pdr test?

[33mcommit 94741e4e21830902ff6e00efa26764ad599826fa[m
Merge: aaeb252 0f30f77
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu Mar 14 17:28:24 2013 +0100

    Merge branch 'master' of bitbucket.org:mweyn/dash7-open-source-stack

[33mcommit aaeb25249792c549f0bc6b08691760dd9a481c67[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu Mar 14 17:28:05 2013 +0100

    removed timout timer in dll, only rf timeout in phy is used with callback to dll

[33mcommit 0f30f77d51a44a51d93a7c7fac36850c1e077b29[m
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Thu Mar 14 16:47:20 2013 +0100

    changed name of communication_test

[33mcommit 08001bf61a6d05fbea28e80e38a6d07a6bbab1c8[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu Mar 14 16:22:11 2013 +0100

    moved timer_init to dll_init

[33mcommit 4135010b245fbf9f818132c7a5e3723ad5344c08[m
Merge: 7e612e4 b57c10b
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Thu Mar 14 14:06:43 2013 +0100

    Merge branch 'master' of https://bitbucket.org/mweyn/dash7-open-source-stack/congestion_ctrl

[33mcommit 7e612e4c9eaebcd0cc1b9334dfefdb9a7937df5f[m
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Thu Mar 14 14:02:19 2013 +0100

    fix pdr_test

[33mcommit b57c10b7a078a80b46fa854ede26f2a6cb004f60[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed Mar 13 17:06:39 2013 +0100

    adapted mac test with new code

[33mcommit 21d51f16ee5249b1f826baf4d42158c1dad70974[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed Mar 13 17:02:52 2013 +0100

    merged congestion_ctrl

[33mcommit 3dc36f2dab3ab3f32791790d5f9fb07942b05704[m
Merge: 3c8c62b ba07fcc
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed Mar 13 16:33:12 2013 +0100

    Merge branch 'master' into congestion_ctrl

[33mcommit 3c8c62bfcee8975ef50001f47ac0e05ca29886a0[m
Merge: 58ada73 d9861d6
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed Mar 13 16:31:52 2013 +0100

    merged master

[33mcommit ba07fcc2e31ba8e32d9cb7729e0740be720daafe[m
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Fri Mar 8 15:18:44 2013 +0100

    pdr_test with enabled FEC (rx not working)

[33mcommit 2e31eee0bb814ae9ed795eebc9698b98e785a64b[m
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Fri Mar 8 10:30:09 2013 +0100

    pdr_logger with correct filename for Windows

[33mcommit f3949dc551ab42ad6d9b237d3626b32150d9530f[m
Merge: da01b8b fd033d7
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Wed Mar 6 13:24:10 2013 +0100

    Merge branch 'fec'
    
    Conflicts:
    	d7aoss/phy/cc430/cc430_phy.c

[33mcommit fd033d7d235b9f003b1e5540521d4840c61689e2[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Wed Mar 6 13:20:06 2013 +0100

    Fec bugfixes (rssi now works with fec)

[33mcommit da01b8bb724f22dfdaa8dbb5461278aaaa5c7efc[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Wed Mar 6 08:31:37 2013 +0100

    Rssi Bugfix

[33mcommit aa2ec102d8c97ccbdc6445aa2e2f5f7c7d5a8da8[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Wed Mar 6 00:18:16 2013 +0100

    Rssi Bugfix

[33mcommit 1097af53225a4734d58e6b102fdd77814ac7def1[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Tue Mar 5 23:06:01 2013 +0100

    Fec Working!

[33mcommit cf9f6e041946330e107ef2aecadc07da14b315f0[m
Merge: 8452e41 4200c6a
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Tue Mar 5 21:13:53 2013 +0100

    Merge branch 'master' into fec

[33mcommit 4200c6ae3cb0d1db16705fb229f4335701561965[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Tue Mar 5 21:11:39 2013 +0100

    Rssi value is int16

[33mcommit e041212ae526ddc6a235863fad224bacacbf8daa[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Tue Mar 5 21:10:13 2013 +0100

    Fixed Get_RSSI()

[33mcommit 8849c0ddc817ebe7383436f19844a90dee300b8a[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Tue Mar 5 20:28:19 2013 +0100

    Enabled Clock_Init(), tested uart, works fine.

[33mcommit 5207d2d34157c5779b47c638b2c35c538c19c6b9[m
Author: Glenn Ergeerts <glenn.ergeerts@artesis.be>
Date:   Tue Mar 5 10:26:45 2013 +0100

    log raw data as well instead of only statistics

[33mcommit dd0ab59ab340d5bac3d6e7c186204bd43fd79955[m
Author: Glenn Ergeerts <glenn.ergeerts@artesis.be>
Date:   Tue Mar 5 09:34:08 2013 +0100

    save testresults in separate directory

[33mcommit 8452e41daf4c7ea3f3773f575c935d23edb8e231[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Mon Mar 4 22:08:52 2013 +0100

    Fec encoding speed improvements (313 cycles)

[33mcommit 2f4697d3ff0eb25f929d1ca9df62e78178a0623d[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Mon Mar 4 20:03:13 2013 +0100

    Fec decode working (430 cycles per input bit)

[33mcommit 2639e0809e2c7cb34b86a4f47b309b679bae82a3[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Mon Mar 4 15:35:16 2013 +0100

    added cc430f5135

[33mcommit c7971d55050c3bdfdb82c4e22abeb1250eedb2c0[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Sun Mar 3 21:44:23 2013 +0100

    Tested encoding speed: currently 28.707 cycles per output bit

[33mcommit 4b47605c45b8ff1af15f0430668820d32f9a7a35[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Sun Mar 3 21:30:51 2013 +0100

    Interleaving bugfix

[33mcommit 6b41cf2a75d7b5b0e7479abf33c7833fb58cb4b6[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Sun Mar 3 19:19:00 2013 +0100

    Fec nearly finished, next up debugging and speed tests

[33mcommit 517b3d51dfd29cdd432d7fe4a48dae51b166d94c[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Sat Mar 2 17:50:08 2013 +0100

    Fec encoding speed improvements

[33mcommit 58ada736004e3cd5cc750cb602c1dd011862e9ad[m
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Fri Mar 1 16:06:16 2013 +0100

    communication_test main

[33mcommit 46bad88111c1c26cd4c6175ad7a3c650f194073e[m
Author: unknown <dragan.subotic@student.artesis.be>
Date:   Fri Mar 1 16:00:41 2013 +0100

    Clean up and new communication_test

[33mcommit 4fff8ed045decee9033e0b7974b0bd6320bd0145[m
Merge: 8504b18 d9861d6
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Thu Feb 28 22:12:56 2013 +0100

    Merge branch 'master' into fec
    
    Conflicts:
    	d7aoss/phy/cc430/cc430_phy.c

[33mcommit d9861d60fe73639c87987eb36ee4f24412c39f86[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Thu Feb 28 21:41:27 2013 +0100

    Phy modification, solved potential problem

[33mcommit 8cec951766963a3c11dea6f7c398fcd3225f7836[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Thu Feb 28 21:40:12 2013 +0100

    Bugfix rx_cfg.length must be set to 0

[33mcommit b116fd37e5b3e167cf166e194bbe7ebec891cd33[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Thu Feb 28 19:54:49 2013 +0100

    MCLK = 16Mhz, SMCLK = 1Mhz

[33mcommit 66b5ddbdb190fe68aa08ef153f1866af20ef41ed[m
Author: Glenn Ergeerts <glenn.ergeerts@artesis.be>
Date:   Thu Feb 28 17:05:48 2013 +0100

    compile fix

[33mcommit 288532f966c6a18c54b6035aa2968997eb3edca7[m
Author: Glenn Ergeerts <glenn.ergeerts@artesis.be>
Date:   Thu Feb 28 17:05:31 2013 +0100

    TODO: increasing clock speed breaks UART, commented for now

[33mcommit 67bc28b6602db7e8f22fd5f01e1fd4a5655c1a8c[m
Merge: c718e75 9a1b95b
Author: Glenn Ergeerts <glenn.ergeerts@artesis.be>
Date:   Thu Feb 28 17:04:08 2013 +0100

    Merge branch 'master' of bitbucket.org:mweyn/dash7-open-source-stack
    
    Conflicts:
    	examples/noise_test/main.c

[33mcommit 363cd179d6790bbb3e9323ffd2784e08f90530f5[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Thu Feb 28 17:01:28 2013 +0100

    fixing phy  and mac to support background frames

[33mcommit 8504b18337f791aea1f432ebcb6dc7c83a5f7b4f[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Wed Feb 27 20:11:06 2013 +0100

    Fec update

[33mcommit f9f2b017dfab4609503a8c0846438223ecce27b3[m
Author: Alexander Hoet <alexanderhoet@gmail.com>
Date:   Wed Feb 27 18:43:37 2013 +0100

    Major overhaul of fec encoding

[33mcommit 35c57de6b5374a76640f36d98562f63b37b6c619[m
Author: Maarten Weyn <maarten.weyn@artesis.be>
Date:   Wed Feb 27 16:45:39 2013 +0100

    adding network layer

[33mcommit c718e75da4065a48baf672ae491c5fa6d9299198[m
Author: Glenn Ergeerts <glenn.ergeerts@artesis.be>
Date:   Wed Feb 27 15:40:43 2013 +0100

    scan all channels

[33mcommit 9a1b95b14646cb25fca3c730a7e51ca5ca62472a[m
Merge: bdb25fa 284adc7
Author: Glenn Ergeerts <glenn.ergeerts@artesis.be>
Date:   Wed Feb 27 14:39:09 2013 +0100

    Merge branch 'master' of bitbucket.org:mweyn/dash7-open-source-stack

[33mcommit bdb25fa1f8e8cadc592997a6b6b486981d8518e7[m
Author: Glenn Ergeerts <glenn.ergeerts@artesis.be>
Date:   Wed Feb 27 14:38:51 2013 +0100

    check sync word and limit to a specific slave mac address to increase robustness
