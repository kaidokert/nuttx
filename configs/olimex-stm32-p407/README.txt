README
======

The NuttX configuration for the Olimex STM32-P407 is derives more or less
directly from the Olimex STM32-P207 board support.  The P207 and P407 seem
to share the same board design.  Other code comes from the STM3240G board
support (which has the same crystal and clocking) and from the STM32 F4
Discovery (which has the same STM32 part)

Board Support
=============

The following peripherals are available in this configuration.

 - LEDs:       show the sytem status

 - Buttons:    TAMPER-button, WKUP-button, J1-Joystick (consists of RIGHT-,
               UP-, LEFT-, DOWN-, and CENTER-button). Built in app
               'buttons' works.

 - ADC:        ADC1 samples the red trim potentiometer AN_TR
               Built in app 'adc' works.

 - USB-FS-OTG: enabled but not really tested, since there is only a
               USB-A-connector (host) connected to the full speed µC inputs.
               The other connector (device) is connected to the high speed µC
               inputs, but it seems that NuttX has currently no driver
               for it.

 - CAN:        Built in app 'can' works, but apart from that not really tested.

 - Ethernet:   Ping to other station on the network works.

microSD Card Interface
======================

  microSD Connector
  -----------------

    ----------------- ----------------- ------------------------
    SD/MMC CONNECTOR        BOARD        GPIO CONFIGURATION(s
    PIN SIGNAL             SIGNAL          (no remapping)
    --- ------------- ----------------- -------------------------
    1   DAT2/RES      SD_D2/USART3_TX/  PC10 GPIO_SDIO_D2
                      SPI3_SCK
    2   CD/DAT3/CS    SD_D3/USART3_RX/  PC11 GPIO_SDIO_D3
                      SPI3_MISO
    3   CMD/DI        SD_CMD            PD2  GPIO_SDIO_CMD
    4   VDD           N/A               N/A
    5   CLK/SCLK      SD_CLK/SPI3_MOSI  PC12 GPIO_SDIO_CK
    6   VSS           N/A               N/A
    7   DAT0/D0       SD_D0/DCMI_D2     PC8  GPIO_SDIO_D0
    8   DAT1/RES      SD_D1/DCMI_D3     PC9  GPIO_SDIO_D1
    --- ------------- ----------------- -------------------------

    NOTES:
    1. DAT4, DAT4, DAT6, and DAT7 not connected.
    2. There are no alternative pin selections.
    3. There is no card detect (CD) GPIO input so we will not
       sense if there is a card in the SD slot or not.  This will
       make usage very awkward.

  Configuration
  -------------

  Enabling SDIO-based MMC/SD support:

    System Type->STM32 Peripheral Support
      CONFIG_STM32_SDIO=y                      : Enable SDIO support
      CONFIG_STM32_DMA2=y                      : DMA2 is needed by the driver

    Device Drivers -> MMC/SD Driver Support
      CONFIG_MMCSD=y                           : Enable MMC/SD support
      CONFIG_MMSCD_NSLOTS=1                    : One slot per driver instance
      # CONFIG_MMCSD_HAVECARDDETECT is not set : No card-detect GPIO
      # CONFIG_MMCSD_MMCSUPPORT is not set     : Interferes with some SD cards
      # CONFIG_MMCSD_SPI is not set            : No SPI-based MMC/SD support
      CONFIG_MMCSD_SDIO=y                      : SDIO-based MMC/SD support
      CONFIG_MMCSD_MULTIBLOCK_DISABLE=y        : Disable to keep things simple
      CONFIG_SDIO_DMA=y                        : Use SDIO DMA
      # CONFIG_SDIO_BLOCKSETUP is not set      : (not implemented)

    Library Routines
      CONFIG_SCHED_WORKQUEUE=y                 : Driver needs work queue support

    Application Configuration -> NSH Library
      CONFIG_NSH_ARCHINIT=y                    : NSH board-initialization

    Using the SD card
    -----------------

    1. Since there is no CD GPIO pin, the firmware sill not know if there is
       a card in the SD slot or not.  It will assume that there is and attempt
       to mount the SD card on power-up.  If there is no SD card in the card
       slot, there will be a long delay during initialization as the firmware
       attempts to query the non-existent card, timeout, and retry.

    2. After booting, an SDIO device will appear as /dev/mmcsd0

    3. If you try mounting an SD card with nothing in the slot, the
       mount will fail:

         nsh> mount -t vfat /dev/mmcsd0 /mnt/sdcard
         nsh: mount: mount failed: 19

    STATUS:
    -------
    2017-01-28:  There is no card communication.  All commands to the SD card timeout.

Configurations
==============

Information Common to All Configurations
----------------------------------------
Each Olimex STM32-P407 configuration is maintained in a sub-directory and can be
selected as follow:

    cd tools
    ./configure.sh olimex-stm32-p407/<subdir>
    cd -
    . ./setenv.sh

Where <subdir> is one of the configuration sub-directories listed in the
following section.

Before sourcing the setenv.sh file above, you should examine it and perform
edits as necessary so that TOOLCHAIN_BIN is the correct path to the directory
than holds your toolchain binaries.

And then build NuttX by simply typing the following.  At the conclusion of
the make, the nuttx binary will reside in an ELF file called, simply, nuttx.

  make oldconfig
  make

    NOTES:

    1. This configuration uses the mconf-based configuration tool.  To
       change this configurations using that tool, you should:

       a. Build and install the kconfig-mconf tool.  See nuttx/README.txt
          see additional README.txt files in the NuttX tools repository.

       b. Execute 'make menuconfig' in nuttx/ in order to start the
          reconfiguration process.

    2. Serial Output

       This configuraiont produces all of its test output on the serial
       console.  This configuration has USART3 enabled as a serial console.
       This is the connector labeled RS232_2.  This can easily be changed
       by reconfiguring with 'make menuconfig'.

    3. Toolchain

       By default, the host platform is set to be Linux using the NuttX
       buildroot toolchain. The host and/or toolchain selection can easily
       be changed with 'make menuconfig'.

    4. Note that CONFIG_STM32_DISABLE_IDLE_SLEEP_DURING_DEBUG is enabled so
       that the JTAG connection is not disconnected by the idle loop.

Configuration sub-directories
-----------------------------

The <subdir> that is provided above as an argument to the tools/configure.sh
must be is one of the following.

  knsh:

    This is identical to the nsh configuration below except that NuttX
    is built as a PROTECTED mode, monolithic module and the user applications
    are built separately.

    It is recommends to use a special make command; not just 'make' but make
    with the following two arguments:

        make pass1 pass2

    In the normal case (just 'make'), make will attempt to build both user-
    and kernel-mode blobs more or less interleaved.  That actual works!
    However, for me it is very confusing so I prefer the above make command:
    Make the user-space binaries first (pass1), then make the kernel-space
    binaries (pass2)

    NOTES:

    1. At the end of the build, there will be several files in the top-level
       NuttX build directory:

       PASS1:
         nuttx_user.elf    - The pass1 user-space ELF file
         nuttx_user.hex    - The pass1 Intel HEX format file (selected in defconfig)
         User.map          - Symbols in the user-space ELF file

       PASS2:
         nuttx             - The pass2 kernel-space ELF file
         nuttx.hex         - The pass2 Intel HEX file (selected in defconfig)
         System.map        - Symbols in the kernel-space ELF file

       The J-Link programmer will except files in .hex, .mot, .srec, and .bin
       formats.

    2. Combining .hex files.  If you plan to use the .hex files with your
       debugger or FLASH utility, then you may need to combine the two hex
       files into a single .hex file.  Here is how you can do that.

       a. The 'tail' of the nuttx.hex file should look something like this
          (with my comments added):

            $ tail nuttx.hex
            # 00, data records
            ...
            :10 9DC0 00 01000000000800006400020100001F0004
            :10 9DD0 00 3B005A0078009700B500D400F300110151
            :08 9DE0 00 30014E016D0100008D
            # 05, Start Linear Address Record
            :04 0000 05 0800 0419 D2
            # 01, End Of File record
            :00 0000 01 FF

          Use an editor such as vi to remove the 05 and 01 records.

       b. The 'head' of the nuttx_user.hex file should look something like
          this (again with my comments added):

            $ head nuttx_user.hex
            # 04, Extended Linear Address Record
            :02 0000 04 0801 F1
            # 00, data records
            :10 8000 00 BD89 01084C800108C8110208D01102087E
            :10 8010 00 0010 00201C1000201C1000203C16002026
            :10 8020 00 4D80 01085D80010869800108ED83010829
            ...

          Nothing needs to be done here.  The nuttx_user.hex file should
          be fine.

       c. Combine the edited nuttx.hex and un-edited nuttx_user.hex
          file to produce a single combined hex file:

          $ cat nuttx.hex nuttx_user.hex >combined.hex

       Then use the combined.hex file with the to write the FLASH image.
       If you do this a lot, you will probably want to invest a little time
       to develop a tool to automate these steps.

  nsh:

    This is the NuttShell (NSH) using the NSH startup logic at
    apps/examples/nsh.

    NOTES:

    1. Kernel Modules / Shared Libraries

       I used this configuration for testing NuttX kernel modules in the
       FLAT build with the following configuration additions to the
       configuration file:

         CONFIG_BOARDCTL_OS_SYMTAB=y
         CONFIG_EXAMPLES_MODULE=y
         CONFIG_EXAMPLES_MODULE_BUILTINFS=y
         CONFIG_EXAMPLES_MODULE_DEVMINOR=0
         CONFIG_EXAMPLES_MODULE_DEVPATH="/dev/ram0"
         CONFIG_FS_ROMFS=y
         CONFIG_LIBC_ARCH_ELF=y
         CONFIG_MODULE=y
         CONFIG_LIBC_MODLIB=y
         CONFIG_MODLIB_ALIGN_LOG2=2
         CONFIG_MODLIB_BUFFERINCR=32
         CONFIG_MODLIB_BUFFERSIZE=128

       Add the following for testing shared libraries in the FLAT
       build:

         CONFIG_LIBC_DLLFCN=y
         CONFIG_EXAMPLES_SOTEST=y
         CONFIG_EXAMPLES_SOTEST_BUILTINFS=y
         CONFIG_EXAMPLES_SOTEST_DEVMINOR=1
         CONFIG_EXAMPLES_SOTEST_DEVPATH="/dev/ram1"

STATUS
======

2016-12-21: This board configuration was ported from the Olimex STM32 P207
  port.  Note that none of the above features have been verified.  USB, CAN,
  ADC, and Ethernet are disabled in the base NSH configuration until they
  can be verified.  These features should be functional but may required
  some tweaks due to the different clock configurations.  The Olimex STM32
  P207 nsh/defconfig would be a good starting place for restoring these
  feature configurations.

  CCM memory is not included in the heap (CONFIG_STM32_CCMEXCLUDE=y) because
  it does no suport DMA, leaving only 128KiB for program usage.

2107-01-23:  Added the the knsh configuration and support for the PROTECTED
  build mode.
