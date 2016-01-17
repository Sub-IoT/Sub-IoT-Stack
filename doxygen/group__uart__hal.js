var group__uart__hal =
[
    [ "fsl_uart_hal.h", "fsl__uart__hal_8h.html", null ],
    [ "UART_SHIFT", "group__uart__hal.html#gad83d258837467b9702bc42da97566ef1", null ],
    [ "uart_bit_count_per_char_t", "group__uart__hal.html#gac4809775c7544386cbc25fcd06f695a2", null ],
    [ "uart_break_char_length_t", "group__uart__hal.html#ga8a5d7a3ed1043bb5a67f1b5d3e9d6453", null ],
    [ "uart_idle_line_select_t", "group__uart__hal.html#ga9082a7f856054fd3a754ec115f91ac6b", null ],
    [ "uart_interrupt_t", "group__uart__hal.html#ga8e2791d1785e2c0036663b1e1be51a14", null ],
    [ "uart_ir_tx_pulsewidth_t", "group__uart__hal.html#ga07f48dd4b36c6a6e57138108697fbbd9", null ],
    [ "uart_iso7816_anack_config_t", "group__uart__hal.html#gadeb67c817ba6ce9e41f78900f6e37d38", null ],
    [ "uart_iso7816_initd_config_t", "group__uart__hal.html#gad0727467c9563bd6b828136cfb82a9e7", null ],
    [ "uart_iso7816_interrupt_t", "group__uart__hal.html#ga7065a93e0e30c94c62a1600fc5b5ba9c", null ],
    [ "uart_iso7816_onack_config_t", "group__uart__hal.html#ga9ac13f74c6f494bec991b63a4188ee38", null ],
    [ "uart_iso7816_transfer_protocoltype_t", "group__uart__hal.html#ga38aa5e8b0fe43f1f18fe083b4b96f6c0", null ],
    [ "uart_operation_config_t", "group__uart__hal.html#ga82fb646c93dcafcd9a096c816ae3d376", null ],
    [ "uart_parity_mode_t", "group__uart__hal.html#ga3d74bf70252b21a0dd19d61587ed320c", null ],
    [ "uart_receiver_source_t", "group__uart__hal.html#ga39d3d0a8736e6a806cb474a4f9f13876", null ],
    [ "uart_singlewire_txdir_t", "group__uart__hal.html#ga5a158787a6da225f49f2372634b670c6", null ],
    [ "uart_status_flag_t", "group__uart__hal.html#gab5a3fa1c858501bdf99d5f46bdb18672", null ],
    [ "uart_status_t", "group__uart__hal.html#ga90effa380d181d660c1bb449977e1535", null ],
    [ "uart_stop_bit_count_t", "group__uart__hal.html#ga1df1fcb3fcd2d2db0b7ea0189fd94554", null ],
    [ "uart_wakeup_method_t", "group__uart__hal.html#ga6aa156a75707a0cd36921c54080e2726", null ],
    [ "_uart_bit_count_per_char", "group__uart__hal.html#ga09ceaf514baf3352c6c4a78155cbfd9d", [
      [ "kUart8BitsPerChar", "group__uart__hal.html#gga09ceaf514baf3352c6c4a78155cbfd9da39a5492922c7775dd7e0ad0cc394d9c7", null ],
      [ "kUart9BitsPerChar", "group__uart__hal.html#gga09ceaf514baf3352c6c4a78155cbfd9da4c94ddc92496671dcc07f56665c423ab", null ]
    ] ],
    [ "_uart_break_char_length", "group__uart__hal.html#gafc836c7460339592ca74ec409835dabb", [
      [ "kUartBreakChar10BitMinimum", "group__uart__hal.html#ggafc836c7460339592ca74ec409835dabba00828d8cd2c2715b3a32f704af22c98c", null ],
      [ "kUartBreakChar13BitMinimum", "group__uart__hal.html#ggafc836c7460339592ca74ec409835dabba2b24dc75d0a4ec4945cc3ec466e0c339", null ]
    ] ],
    [ "_uart_idle_line_select", "group__uart__hal.html#gafcbb35202aa124845649edb0607d994d", [
      [ "kUartIdleLineAfterStartBit", "group__uart__hal.html#ggafcbb35202aa124845649edb0607d994daf6156450600a19995b4c47d0b4358d9d", null ],
      [ "kUartIdleLineAfterStopBit", "group__uart__hal.html#ggafcbb35202aa124845649edb0607d994da564a13c59cf3aa1cbf7322eb1c0930a9", null ]
    ] ],
    [ "_uart_interrupt", "group__uart__hal.html#ga0241bd5ddb8629625e7facd8da941fd9", [
      [ "kUartIntRxActiveEdge", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9ae8c18965d22c094e4a374677e98d8497", null ],
      [ "kUartIntTxDataRegEmpty", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9ac830ccec89a9106526097f1eeb63b871", null ],
      [ "kUartIntTxComplete", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9a58c07fbae21ccd3fb368a575f689ed19", null ],
      [ "kUartIntRxDataRegFull", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9ac1fcf894ef73e69e93936efc1dba775a", null ],
      [ "kUartIntIdleLine", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9aa462b837c307f2425e4e24f3e5464c9a", null ],
      [ "kUartIntRxOverrun", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9ae016d68c7014f337b503eb1c98761721", null ],
      [ "kUartIntNoiseErrFlag", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9ae1c9ca936836e9cc4757c396c823c593", null ],
      [ "kUartIntFrameErrFlag", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9a1590d1fa1cffb3d9bb2f9594f6cf6713", null ],
      [ "kUartIntParityErrFlag", "group__uart__hal.html#gga0241bd5ddb8629625e7facd8da941fd9aa86f9cbb3556c7801f4986a94306a066", null ]
    ] ],
    [ "_uart_ir_tx_pulsewidth", "group__uart__hal.html#ga9bf0284b094edf04f6b91db68f20ed7d", [
      [ "kUartIrThreeSixteenthsWidth", "group__uart__hal.html#gga9bf0284b094edf04f6b91db68f20ed7dabc3065fe188c6498365f33b40db32414", null ],
      [ "kUartIrOneSixteenthWidth", "group__uart__hal.html#gga9bf0284b094edf04f6b91db68f20ed7da4af56197dd644086548f53f873add6c2", null ],
      [ "kUartIrOneThirtysecondsWidth", "group__uart__hal.html#gga9bf0284b094edf04f6b91db68f20ed7dadd2e9d00bfb3c194fabc51fc0cbb85d4", null ],
      [ "kUartIrOneFourthWidth", "group__uart__hal.html#gga9bf0284b094edf04f6b91db68f20ed7da43b6f8056ad9cef47393f120e741a590", null ]
    ] ],
    [ "_uart_iso7816_anack_config", "group__uart__hal.html#ga21cfaf12b0dc518d563e5bf5ba8f447b", [
      [ "kUartIso7816AnackDisable", "group__uart__hal.html#gga21cfaf12b0dc518d563e5bf5ba8f447bac0c7b407f6e133c7110851ac5bc8d5ae", null ],
      [ "kUartIso7816AnackEnable", "group__uart__hal.html#gga21cfaf12b0dc518d563e5bf5ba8f447ba971ae89164241c2088c557eeb747b5c3", null ]
    ] ],
    [ "_uart_iso7816_initd_config", "group__uart__hal.html#ga7fa96f997a9f9e1b9ac378d88ce193c4", [
      [ "kUartIso7816InitdDisable", "group__uart__hal.html#gga7fa96f997a9f9e1b9ac378d88ce193c4a1923e3bc52842024c87fe2c3a99ee37e", null ],
      [ "kUartIso7816InitdEnable", "group__uart__hal.html#gga7fa96f997a9f9e1b9ac378d88ce193c4a7b8cb357fa149b112c6f57ba2babc8c4", null ]
    ] ],
    [ "_uart_iso7816_interrupt", "group__uart__hal.html#ga4b70dc8f87cd1d8c745b6df8c05a72b3", [
      [ "kUartIntIso7816RxThreasholdExceeded", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3afc5034f268507c66b389eb4866bac4ea", null ],
      [ "kUartIntIso7816TxThresholdExceeded", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3a9a1508c3245aa7b042fe601541f41b35", null ],
      [ "kUartIntIso7816GuardTimerViolated", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3acf8efe8b2a93c07ff3c7f9dd3e62916f", null ],
      [ "kUartIntIso7816AtrDurationTimer", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3a4eef9a87bd2a26d1086312bb9d69296f", null ],
      [ "kUartIntIso7816InitialCharDetected", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3a6cc83946ed985902ce62d63d287e70a6", null ],
      [ "kUartIntIso7816BlockWaitTimer", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3a59d949485be96927321930008327eb75", null ],
      [ "kUartIntIso7816CharWaitTimer", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3a4a64350e2a0b263da71a5e01f18b1bcc", null ],
      [ "kUartIntIso7816WaitTimer", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3a62e23b46447a6472898dae8fce00da3d", null ],
      [ "kUartIntIso7816All", "group__uart__hal.html#gga4b70dc8f87cd1d8c745b6df8c05a72b3a7462934103a6d8e061535f46029ee4d6", null ]
    ] ],
    [ "_uart_iso7816_onack_config", "group__uart__hal.html#ga6676b98769bedcf75583b417697ab8ea", [
      [ "kUartIso7816OnackEnable", "group__uart__hal.html#gga6676b98769bedcf75583b417697ab8eaa50a7a85232a2e1d73832242d4c53fc2c", null ],
      [ "kUartIso7816OnackDisable", "group__uart__hal.html#gga6676b98769bedcf75583b417697ab8eaa3cee93649d0b356e5a0319966e25402c", null ]
    ] ],
    [ "_uart_iso7816_tranfer_protocoltype", "group__uart__hal.html#ga6f04f7fe839c84468261062b2a19b65f", [
      [ "kUartIso7816TransfertType0", "group__uart__hal.html#gga6f04f7fe839c84468261062b2a19b65fa60730b8a5bedeeb20a3ee6465f0f9471", null ],
      [ "kUartIso7816TransfertType1", "group__uart__hal.html#gga6f04f7fe839c84468261062b2a19b65faba9cfb11055c10daf8c409df792219c9", null ]
    ] ],
    [ "_uart_operation_config", "group__uart__hal.html#ga6691a4a7f6d91eab489a02a8d8acbec9", [
      [ "kUartOperates", "group__uart__hal.html#gga6691a4a7f6d91eab489a02a8d8acbec9a103d89ec75d4d974582f99f43bc64117", null ],
      [ "kUartStops", "group__uart__hal.html#gga6691a4a7f6d91eab489a02a8d8acbec9a0fa5187cd9ee9f9cde52d2730959d673", null ]
    ] ],
    [ "_uart_parity_mode", "group__uart__hal.html#gadf9f66755acc340eab030e1a48e35e10", [
      [ "kUartParityDisabled", "group__uart__hal.html#ggadf9f66755acc340eab030e1a48e35e10aaa442d2224e06b4118463ed49f3768d6", null ],
      [ "kUartParityEven", "group__uart__hal.html#ggadf9f66755acc340eab030e1a48e35e10a3f5ca507d31e770da03b9ca473b41469", null ],
      [ "kUartParityOdd", "group__uart__hal.html#ggadf9f66755acc340eab030e1a48e35e10adc98b9d68156ba24739da03a52a0631f", null ]
    ] ],
    [ "_uart_receiver_source", "group__uart__hal.html#gae84554e8367780b93162f4c3a3c46082", [
      [ "kUartLoopBack", "group__uart__hal.html#ggae84554e8367780b93162f4c3a3c46082a4524676c69586c50a07c5eaf4c3c2a99", null ],
      [ "kUartSingleWire", "group__uart__hal.html#ggae84554e8367780b93162f4c3a3c46082a15ae5b502a1ec1196b70b8a3fd9476fb", null ]
    ] ],
    [ "_uart_singlewire_txdir", "group__uart__hal.html#ga18cefb04a659432f96e2166b2205765a", [
      [ "kUartSinglewireTxdirIn", "group__uart__hal.html#gga18cefb04a659432f96e2166b2205765aa204dcc6ef033e1c99037078bada5621c", null ],
      [ "kUartSinglewireTxdirOut", "group__uart__hal.html#gga18cefb04a659432f96e2166b2205765aa1687332c34852bebdd0f36334a2b2bb3", null ]
    ] ],
    [ "_uart_status", "group__uart__hal.html#gacef40dc8e8ac174bfe40ebcbc980f84b", [
      [ "kStatus_UART_Success", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba0c01e5e7197f20f6d97b98c706677f54", null ],
      [ "kStatus_UART_Fail", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba73663e6bd68c644e244e3b4f8ddf5581", null ],
      [ "kStatus_UART_BaudRateCalculationError", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba546f54d8fa70c1025dbf144d6a76823b", null ],
      [ "kStatus_UART_RxStandbyModeError", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba2796b1d04991f5a137d6259740480eff", null ],
      [ "kStatus_UART_ClearStatusFlagError", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84bafc5d007255989951b6dc1f8c252a929d", null ],
      [ "kStatus_UART_TxNotDisabled", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba2d1007e79b0c5ade7a69fb0171e6043c", null ],
      [ "kStatus_UART_RxNotDisabled", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba0664917ca33d447556d47cdd83d84190", null ],
      [ "kStatus_UART_TxOrRxNotDisabled", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba204d6435aa9766afea6c25c3c3d60c50", null ],
      [ "kStatus_UART_TxBusy", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba19ddee4fe5963467600028b78fb468e7", null ],
      [ "kStatus_UART_RxBusy", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba68cf23f981c4b85f82291163fbb2a5e6", null ],
      [ "kStatus_UART_NoTransmitInProgress", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84badce3eb43e9dccf475f674961c2c8d903", null ],
      [ "kStatus_UART_NoReceiveInProgress", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba40c83f3edaece9f1fe165c004f87566c", null ],
      [ "kStatus_UART_Timeout", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba1fef1b2a4a60c45c7fc329bbe8c1e34d", null ],
      [ "kStatus_UART_Initialized", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84bac96f0c7346bac9c27131a2fdd37b5f10", null ],
      [ "kStatus_UART_NoDataToDeal", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84ba51c44f98ebe910b925384d916a62e279", null ],
      [ "kStatus_UART_RxOverRun", "group__uart__hal.html#ggacef40dc8e8ac174bfe40ebcbc980f84baa116d6bf491ee0ccc662f1fc431629a3", null ]
    ] ],
    [ "_uart_status_flag", "group__uart__hal.html#gaa4ec9f908b1b8e958c99ddff14373882", [
      [ "kUartTxDataRegEmpty", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882aa64266a26f7484bf13f166f5748f59f1", null ],
      [ "kUartTxComplete", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882a279387cdd7ef50baa10a884958b4167e", null ],
      [ "kUartRxDataRegFull", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882a51209510afba63ec18ff6d4b2568696b", null ],
      [ "kUartIdleLineDetect", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882a6501d94c6320621bfc570529de6f7441", null ],
      [ "kUartRxOverrun", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882a218febaed0bb1045e42432024dd992d9", null ],
      [ "kUartNoiseDetect", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882acfbb3038b0b81022de7d3600fd0a3255", null ],
      [ "kUartFrameErr", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882a3d96d49ccbc30ad8a7cae0ff64a7c8ff", null ],
      [ "kUartParityErr", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882a744b74085a23e42ecdfdaace08ef176b", null ],
      [ "kUartRxActiveEdgeDetect", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882aaea3e47f7f337f95246c3decdc99abd2", null ],
      [ "kUartRxActive", "group__uart__hal.html#ggaa4ec9f908b1b8e958c99ddff14373882aca3a161522a90f47010844017a98085d", null ]
    ] ],
    [ "_uart_stop_bit_count", "group__uart__hal.html#ga3c656a4365cab1185398ff953272091e", [
      [ "kUartOneStopBit", "group__uart__hal.html#gga3c656a4365cab1185398ff953272091ea792ceeb3ebaaf5b2529423fe8e178d44", null ],
      [ "kUartTwoStopBit", "group__uart__hal.html#gga3c656a4365cab1185398ff953272091ea27898a03ccf45fe91fddfff83be60cea", null ]
    ] ],
    [ "_uart_wakeup_method", "group__uart__hal.html#gac4479526dd3c0c5406e7c553e2c797d5", [
      [ "kUartIdleLineWake", "group__uart__hal.html#ggac4479526dd3c0c5406e7c553e2c797d5a09721a9294ef5eadb137e4a0a9744a58", null ],
      [ "kUartAddrMarkWake", "group__uart__hal.html#ggac4479526dd3c0c5406e7c553e2c797d5a51f8c90fb648dbd7d2f684d46f05975b", null ]
    ] ],
    [ "UART_HAL_ClearStatusFlag", "group__uart__hal.html#gadfa538ebba0365d5480021e8b9abf07a", null ],
    [ "UART_HAL_ConfigIdleLineDetect", "group__uart__hal.html#gae032bd60adabfe550533467ee6fbec70", null ],
    [ "UART_HAL_Getchar", "group__uart__hal.html#gab1d05b425846e8c7efe0b238d7873f37", null ],
    [ "UART_HAL_Getchar9", "group__uart__hal.html#gaeda41db4209b4587b51da85396d1eaba", null ],
    [ "UART_HAL_GetIntMode", "group__uart__hal.html#ga53444ddd41537d439921aec8afd80264", null ],
    [ "UART_HAL_GetStatusFlag", "group__uart__hal.html#gaa6485c490ecd09d0ec42303536baa599", null ],
    [ "UART_HAL_Init", "group__uart__hal.html#gaa17d9f21508fb0fed1804f1a07d4fe72", null ],
    [ "UART_HAL_Putchar", "group__uart__hal.html#ga242f62e5ce130b7a9d03cd6ccfbb4759", null ],
    [ "UART_HAL_Putchar9", "group__uart__hal.html#gadeeef90d515b827c99355005a2000604", null ],
    [ "UART_HAL_PutReceiverInStandbyMode", "group__uart__hal.html#gaca0ade16fc256388ef51513bdd5c89e3", null ],
    [ "UART_HAL_ReceiveDataPolling", "group__uart__hal.html#ga99c75a7e88940b286c233bc6412da334", null ],
    [ "UART_HAL_SendDataPolling", "group__uart__hal.html#gaf932d0b46b6e10a2a053b175326322ce", null ],
    [ "UART_HAL_SetBaudRate", "group__uart__hal.html#ga11a93d335f97424a285be32fec925c2c", null ],
    [ "UART_HAL_SetBaudRateDivisor", "group__uart__hal.html#ga2a0d0078aa82155875cc30f46df7f005", null ],
    [ "UART_HAL_SetIntMode", "group__uart__hal.html#gad6906bc4d2d6877f6ac64863410972b2", null ],
    [ "UART_HAL_SetMatchAddress", "group__uart__hal.html#gaaf5db8c2310688cac4203a46eef8e4ae", null ],
    [ "UART_HAL_SetParityMode", "group__uart__hal.html#ga802ad49c6b5ab21f4e4967df9a009995", null ]
];